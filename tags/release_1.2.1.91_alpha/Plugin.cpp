/*
  Copyright (C) 2008 Laurent Cozic. All right reserved.
  Use of this source code is governed by a GNU/GPL license that can be
  found in the LICENSE file.
*/

#include "stdafx.h"

#include "Plugin.h"
#include "MiniLaunchBar.h"
#include "FolderItemRenderer.h"
#include "OptionButton.h"

#include "lua_glue/azGlobal.h"
#include "lua_glue/azApplication.h"
#include "lua_glue/azIcon.h"
#include "lua_glue/azOptionPanel.h"
#include "lua_glue/azOptionButton.h"
#include "lua_glue/azMenu.h"
#include "lua_glue/azShortcut.h"



Plugin::Plugin() {
  enabled_ = true;
  initiallyEnabled_ = enabled_;
  
  L = NULL;
}


Plugin::~Plugin() {
  if (L) lua_close(L);

  std::map<std::pair<wxObject*, int>, wxArrayString*>::iterator it = eventRegister_.begin();

  for(; it != eventRegister_.end(); ++it) {
    wxArrayString* v = it->second;
    wxDELETE(v);
  }
}


void Plugin::SetInitiallyEnabled(bool enabled) {
  initiallyEnabled_ = enabled;
}


bool Plugin::WasInitiallyEnabled() { return initiallyEnabled_; }
wxString Plugin::GetName() { return name_; }
wxString Plugin::GetUUID() { return uuid_; }
bool Plugin::IsEnabled() { return enabled_; }
void Plugin::Disable() { Enable(false); }


void Plugin::Enable(bool enable) {
  if (enable == enabled_) return;

  enabled_ = enable;
}


void Plugin::LoadPluginXml(const wxString& xmlFilePath) {
  ILOG(_T("Loading: ") + xmlFilePath);

  if (!wxFileName::FileExists(xmlFilePath)) {
    ILOG(_T("Plugin::LoadPluginXml: 'plugin.xml' is missing: ") + xmlFilePath + _T(" Using default settings"));
    return;
  }

  TiXmlDocument doc(xmlFilePath.mb_str());
  doc.LoadFile(TIXML_ENCODING_UTF8);

  TiXmlElement* root = doc.FirstChildElement("Plugin");
  if (!root) {
    ELOG(_T("Plugin::LoadPluginXml: Could not load XML. No 'Plugin' element found."));
    return;
  }

  TiXmlHandle handle(root);

  name_ = XmlUtil::ReadElementText(handle, "Name");
  uuid_ = XmlUtil::ReadElementText(handle, "UUID");
}


void Plugin::LoadMetadata(const wxString& folderPath) {
  wxString xmlFilePath = folderPath + wxFileName::GetPathSeparator() + _T("plugin.xml");
  LoadPluginXml(xmlFilePath);

  wxFileName folderPathFN(folderPath);
  if (name_ == wxEmptyString) name_ = folderPathFN.GetName();
  if (uuid_ == wxEmptyString) uuid_ = name_;
}


void Plugin::Load(const wxString& folderPath) {
  wxString luaFilePath = folderPath + wxFileName::GetPathSeparator() + _T("main.lua");

  if (!wxFileName::FileExists(luaFilePath)) {
    ELOG(_T("Plugin::Load: 'main.lua' is missing for plugin: ") + folderPath);
    return;
  }

  L = lua_open();

  luaopen_base(L);
  luaopen_table(L);
  luaopen_string(L);
  luaopen_math(L);

  lua_register(L, "trace", azPrint);

  int error = luaL_loadfile(L, luaFilePath.mb_str());

  if (error) {
    LuaUtil::LogError(error);
    return;
  }

  Lunar<azApplication>::Register(L);
  Lunar<azIcon>::Register(L);
  Lunar<azMenu>::Register(L);
  Lunar<azShortcut>::Register(L);
  Lunar<azOptionButton>::Register(L);
  Lunar<azOptionPanel>::Register(L);
  
  lua_pushliteral(L, "appetizer");
  Lunar<azApplication>::push(L, wxGetApp().GetPluginManager()->luaApplication);
  lua_settable(L, LUA_GLOBALSINDEX);

  lua_pushliteral(L, "optionPanel");
  Lunar<azOptionPanel>::push(L, wxGetApp().GetPluginManager()->luaOptionPanel);
  lua_settable(L, LUA_GLOBALSINDEX);

  error = lua_pcall(L, 0, 0, 0);

  if (error) {
    const char* errorString = lua_tostring(L, -1);
    luaHost_logError(wxString(errorString, wxConvUTF8), _T("Plugin::LoadFile"));
  }
}


void Plugin::AddEventListener(wxObject* object, int eventId, const wxString& functionName) {
  std::pair<wxObject*, int> pair(object, eventId);

  wxArrayString* functionNames = eventRegister_[pair];

  if (!functionNames) {
    functionNames = new wxArrayString();
    eventRegister_[pair] = functionNames;
  }

  functionNames->Add(functionName);
}


void Plugin::AddEventListener(wxObject* object, const wxString& eventName, const wxString& functionName) {
  int eventId = wxGetApp().GetPluginManager()->GetEventIdByName(eventName);
  AddEventListener(object, eventId, functionName);
}


void Plugin::DispatchEvent(wxObject* sender, const wxString& eventName, LuaHostTable arguments) {
  int eventId = wxGetApp().GetPluginManager()->GetEventIdByName(eventName);
  DispatchEvent(sender, eventId, arguments);
}


template <class hostObjectT, class lunarObjectT>
bool luaConvertAndPushAsWrapper(lua_State* L, wxObject* o) {
  hostObjectT* asType = dynamic_cast<hostObjectT*>(o);
  if (asType) {
    Lunar<lunarObjectT>::push(L, new lunarObjectT(asType), true);
    return true;
  }
  return false;
}


template <class T>
bool luaPushAsWrapper(lua_State* L, wxObject* o) {
  T* asType = dynamic_cast<T*>(o);
  if (asType) {
    Lunar<T>::push(L, asType, true);
    return true;
  }
  return false;
}


void Plugin::DispatchEvent(wxObject* sender, int eventId, LuaHostTable arguments) {
  std::pair<wxObject*, int> pair(sender, eventId);

  wxArrayString* functionNames = eventRegister_[pair];
  if (!functionNames) return;

  for (int i = 0; i < functionNames->Count(); i++) {
    wxString n = (*functionNames)[i];

    lua_getfield(L, LUA_GLOBALSINDEX, n.mb_str());
    lua_createtable(L, arguments.size(), 0);
    int tableIndex = lua_gettop(L);
    
    LuaHostTable::iterator it = arguments.begin();
    for(; it != arguments.end(); ++it) {
      wxString k = it->first;
      LuaHostTableItem* hostTableItem = it->second;

      wxObject* value = hostTableItem->value;
      LuaHostTableItemType valueType = hostTableItem->valueType;

      lua_pushstring(L, k.mb_str());

      bool done = true;

      if (valueType == LHT_boolean) {

        lua_pushboolean(L, *((wxString*)value) != _T("0"));

      } else if (valueType == LHT_integer) {

        wxString* s = (wxString*)value;
        long l; if (!s->ToLong(&l)) l = 0;
        lua_pushinteger(L, (int)l);

      } else if (valueType == LHT_string) {

        wxString* s = (wxString*)value;
        lua_pushstring(L, s->ToUTF8());

      } else if (valueType == LHT_wxObject) {

        done = luaPushAsWrapper<azIcon>(L, value);
        if (!done) done = luaPushAsWrapper<azMenu>(L, value);
        if (!done) done = luaPushAsWrapper<azOptionButton>(L, value);
        if (!done) done = luaPushAsWrapper<azOptionPanel>(L, value);
        if (!done) done = luaPushAsWrapper<azShortcut>(L, value);
        if (!done) done = luaPushAsWrapper<azApplication>(L, value);

      } else {

        done = false;

      }

      if (!done) wxLogDebug(_T("[ERROR] Cannot detect type of ") + k);

      lua_settable(L, tableIndex);      
    }

    lua_pushstring(L, "sender");

    bool done = false;

    done = luaConvertAndPushAsWrapper<FolderItemRenderer, azIcon>(L, sender);
    if (!done) done = luaConvertAndPushAsWrapper<wxMenu, azMenu>(L, sender);
    if (!done) done = luaConvertAndPushAsWrapper<OptionButton, azOptionButton>(L, sender);

    if (!done && dynamic_cast<MiniLaunchBar*>(sender)) {
      Lunar<azApplication>::push(L, wxGetApp().GetPluginManager()->luaApplication, true);
      done = true;
    }

    if (!done && dynamic_cast<OptionPanel*>(sender)) {
      Lunar<azOptionPanel>::push(L, wxGetApp().GetPluginManager()->luaOptionPanel, true);
      done = true;
    }

    if (!done) wxLogDebug(_T("[ERROR] Cannot detect type of sender"));

    lua_settable(L, tableIndex);   

    int errorCode = lua_pcall(L, 1, 0, 0);

    if (errorCode) {
      const char* errorString = lua_tostring(L, -1);
      luaHost_logError(wxString(errorString, wxConvUTF8), _T("Plugin::DispatchEvent"));
    }

  }

}


bool Plugin::HandleMenuItemClick(ExtendedMenuItem* menuItem) {
  wxString onClickHandler = menuItem->GetMetadata(_T("plugin_onClick"));
  if (onClickHandler == wxEmptyString) return false;

  lua_getfield(L, LUA_GLOBALSINDEX, onClickHandler.mb_str());

  lua_createtable(L, 1, 0);
  int tableIndex = lua_gettop(L);

  lua_pushstring(L, "menuItemId");
  lua_pushstring(L, menuItem->GetMetadata(_T("plugin_menuItemId")).mb_str());
  lua_settable(L, tableIndex);

  lua_pushstring(L, "menuItemTag");
  lua_pushstring(L, menuItem->GetMetadata(_T("plugin_menuItemTag")).mb_str());
  lua_settable(L, tableIndex);  
  
  int errorCode = lua_pcall(L, 1, 0, 0);

  if (errorCode) {
    const char* errorString = lua_tostring(L, -1);
    luaHost_logError(wxString(errorString, wxConvUTF8), _T("Plugin::DispatchEvent"));
  }

  return true;
}


lua_State* Plugin::GetLuaState() {
  return L;
}