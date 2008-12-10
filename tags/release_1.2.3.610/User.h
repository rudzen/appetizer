﻿/*
  Copyright (C) 2008 Laurent Cozic. All right reserved.
  Use of this source code is governed by a GNU/GPL license that can be
  found in the LICENSE file.
*/

#include "stdafx.h"

#ifndef __User_H
#define __User_H

#include "FolderItem.h"
#include "Enumerations.h"
#include "UserSettings.h"
#include "gui/ShortcutEditorDialog.h"


class User : public wxEvtHandler {

public:

  User();  
  ~User();  
  UserSettings* GetSettings();
  void ScheduleSave();
  void Save(bool force = false);
  void Load();

  void GetShortcutsFromFolder(const wxString& folderPath, wxArrayString* result);
  
  void PortableAppsFormatSynchronization();
  void StartMenuSynchronization();
  void QuickLaunchSynchronization();

  int EditFolderItem(FolderItem* folderItem);
  FolderItem* EditNewFolderItem(FolderItem* parent, bool isGroup = false);
  FolderItem* AddNewFolderItemFromPath(FolderItem* parent, wxString folderItemPath);

  void AddAutoAddExclusion(const wxString& filePath);
  bool IsAutoAddExclusion(const wxString& filePath);
  wxArrayString GetAutoAddExclusions();
  void SetAutoAddExclusions(wxArrayString& arrayString);

  void BatchAddFolderItems(const wxArrayString& filePaths, bool useAutoAddExclusions = false);

  FolderItem* GetRootFolderItem();

  void OnTimer(wxTimerEvent& evt);

private:

  FolderItem* rootFolderItem_;
  wxTimer* scheduledSaveTimer_;
  wxStringList folderItemExclusions_;
  UserSettings* settings_;
  ShortcutEditorDialog* shortcutEditorDialog_;
  wxArrayString autoAddExclusions_;

  DECLARE_EVENT_TABLE()

};


#endif