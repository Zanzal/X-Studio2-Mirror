
///////////////////////////////// ID //////////////////////////////////

#define IDR_MAINFRAME                   100

#define IDR_BACKUP                      119
#define IDR_PROPERTIES                  120
#define IDR_OUTPUT                      121
#define IDR_COMMANDS                    122
#define IDR_GAME_OBJECTS                123
#define IDR_SCRIPT_OBJECTS              124
#define IDR_EDITOR                      125

#define IDR_LANGUAGEVIEW                126
#define IDR_SCRIPTVIEW                  127
#define IDR_PROJECTVIEW                 128
#define IDR_DIFFVIEW                    129
#define IDR_STRINGVIEW                  130

///////////////////////////////// TOOLBARS /////////////////////////////

#define IDT_FILE                        150
#define IDT_EDIT                        151
#define IDT_GAMEDATA                    152
#define IDT_VIEW                        153
#define IDT_MENU_IMAGES                 154

////////////////////////////////// MENUS ///////////////////////////////

#define IDM_EDIT_POPUP                  160
#define IDM_SCRIPTEDIT_POPUP            161
#define IDM_PROJECT_POPUP               162
#define IDM_OUTPUT_POPUP                163
#define IDM_STRINGVIEW_POPUP            164
#define IDM_BACKUP_POPUP                165
#define IDM_GAMEDATA_POPUP              166

///////////////////////////////// BITMAPS //////////////////////////////

#define IDB_EDITOR_GREY                 179
#define IDB_PROJECT_ICONS               181
#define IDB_OUTPUT_ICONS                182
#define IDB_GAMEDATA_ICONS              183
#define IDB_LANGUAGE_ICONS              184
#define IDB_RICH_BUTTON                 185
#define IDB_USER_IMAGES                 186
#define IDB_NEWDOCUMENT_ICONS           187
#define IDB_SPLASH                      188
#define IDB_BACKUP_ICONS                189
#define IDB_EXPORT_PROJECT              190
#define IDB_IMPORT_PROJECT              191
#define IDB_EDIT_ARGUMENT               192

////////////////////////////////// ICONS ///////////////////////////////


///////////////////////////////// COMMANDS /////////////////////////////

// File Menu
#define ID_FILE_BROWSE                  250
#define ID_FILE_SAVE_ALL                251
#define ID_FILE_EXPORT                  252
#define ID_FILE_COMMIT                  253
#define ID_FILE_QUICK_COMMIT            254
#define ID_FILE_PROJECT_CLOSE           255
#define ID_FILE_PROJECT_SAVE            256
#define ID_FILE_PROJECT_SAVE_AS         257

// Edit Menu
#define ID_EDIT_INSERT                  258
#define ID_EDIT_PREFERENCES             259

// Test menu
#define ID_TEST_RUN_ALL                 260
#define ID_TEST_COMPILE                 261

// View menu
#define ID_VIEW_PROJECT                 262
#define ID_VIEW_SCRIPT_OBJECTS          263
#define ID_VIEW_GAME_OBJECTS            264
#define ID_VIEW_CONSOLE                 265
#define ID_VIEW_COMMANDS                266
#define ID_VIEW_OUTPUT                  267
#define ID_VIEW_BACKUP                  268
#define ID_VIEW_PROPERTIES              269
#define ID_VIEW_STRING_LIBRARY          270
#define ID_VIEW_MEDIA_LIBRARY           271
#define ID_VIEW_CUSTOMIZE               272

// Window menu
#define ID_WINDOW_MANAGER               273

// Help
#define ID_HELP_FORUMS                  274


////////////////////////// CONTEXT MENU COMMANDS ////////////////////////

// PropertiesWnd ContextMenu
#define ID_EXPAND_ALL                   300
#define ID_SORTPROPERTIES               301
#define ID_INSERT_ARGUMENT              302
#define ID_REORDER_ARGUMENT_UP          303
#define ID_EDIT_ARGUMENT                304
#define ID_REORDER_ARGUMENT_DOWN        305
#define ID_REMOVE_ARGUMENT              306

// ProjectWnd ContextMenu
#define ID_PROJECT_OPEN                 350
#define ID_PROJECT_ADD_FILE             351
#define ID_PROJECT_ADD_FOLDER           352
#define ID_PROJECT_COMMIT               353
#define ID_PROJECT_QUICK_COMMIT         354
#define ID_PROJECT_RENAME               355
#define ID_PROJECT_REMOVE               356
#define ID_PROJECT_DELETE               357
#define ID_PROJECT_PROPERTIES           358

// BackupWnd ContextMenu
#define ID_BACKUP_DIFF                  372
#define ID_BACKUP_REVERT                373
#define ID_BACKUP_DELETE                374

// LanguageEditor Toolbar
#define ID_EDIT_BOLD                    400
#define ID_EDIT_ITALIC                  401
#define ID_EDIT_UNDERLINE               402
#define ID_EDIT_COLOUR                  403
#define ID_EDIT_LEFT                    404
#define ID_EDIT_CENTRE                  405
#define ID_EDIT_RIGHT                   406
#define ID_EDIT_JUSTIFY                 407
#define ID_EDIT_ADD_BUTTON              408
#define ID_VIEW_SOURCE                  409
#define ID_VIEW_EDITOR                  410
#define ID_VIEW_DISPLAY                 411
#define ID_COLOUR_SELECT                412

// ScriptView ContextMenu
#define ID_EDIT_COMMENT                 450
#define ID_EDIT_LOCALIZE                451
#define ID_EDIT_REFACTOR                452
#define ID_EDIT_OPEN_SCRIPT             453
#define ID_EDIT_GOTO_LABEL              454
#define ID_EDIT_VIEW_STRING             455
#define ID_EDIT_INDENT                  456
#define ID_EDIT_OUTDENT                 457
#define ID_EDIT_FORMAT_SEL              458
#define ID_EDIT_FORMAT_DOC              459

// GameData ContextMenu
#define ID_GAMEDATA_INSERT              500
#define ID_GAMEDATA_LOOKUP              501
#define ID_GAMEDATA_RELOAD              502

///////////////////////////////// DIALOGS //////////////////////////////

#define IDD_ABOUTBOX                    700
#define IDD_FINDREPLACE                 701
#define IDD_PROGRESS                    702
#define IDD_GENERAL_PAGE                703
#define IDD_COMPILER_PAGE               704
#define IDD_EDITOR_PAGE                 705
#define IDD_SYNTAX_PAGE                 706
#define IDD_NEW_DOCUMENT                707
#define IDD_SPLASH                      708
#define IDD_REFACTOR                    709
#define IDD_SYMBOL                      710
#define IDD_IMPORT_PROJECT              711
#define IDD_COMMIT                      712
#define IDD_ARGUMENT                    713
#define IDD_EXPORT_PROJECT              714

///////////////////////////////// CONTROLS //////////////////////////////

// Common
#define IDC_IMAGE_STATIC                790
#define IDC_TITLE_STATIC                791
#define IDC_HEADING1_STATIC             792
#define IDC_HEADING2_STATIC             793
#define IDC_HEADING3_STATIC             794

// Script View
#define IDC_SCOPE_COMBO                 800
#define IDC_VARIABLES_COMBO             801
#define IDC_SCRIPT_EDIT                 802
#define IDC_SUGGESTION_LIST             803

// Language View
#define IDC_PAGE_LIST                   800
#define IDC_STRING_LIST                 801
#define IDC_STRING_EDIT                 802

// Diff View
#define IDC_DIFF_EDIT                   800

// Find Dialog
#define IDC_FIND                        800
#define IDC_REPLACE                     801
#define IDC_FIND_ALL                    802
#define IDC_REPLACE_ALL                 803
#define IDC_FIND_COMBO                  804
#define IDC_REPLACE_COMBO               805
#define IDC_TARGET_COMBO                806
#define IDC_CASE_CHECK                  807
#define IDC_WORD_CHECK                  808
#define IDC_REGEX_CHECK                 809
#define IDC_OPTIONS                     810
#define IDC_RESULTS1_RADIO              811
#define IDC_RESULTS2_RADIO              812
#define IDC_FIND_GROUPBOX               813
#define IDC_FIND_RESIZE                 814

// Find Progress Dialog
#define IDC_PROGRESS_BAR                800
#define IDC_PROGRESS_STATIC             801

// Project Dialog
#define IDC_PROJECT_TREE                800

// Backup Window
#define IDC_BACKUP_LIST                 800

// Preferences dialog
#define IDC_PROPERTY_GRID               800

// New document dialog
#define IDC_TEMPLATE_LIST               800
#define IDC_FILENAME_EDIT               801
#define IDC_FOLDER_EDIT                 802
#define IDC_DESCRIPTION_EDIT            803
#define IDC_ADD_PROJECT_CHECK           804

// Commit dialog
#define IDC_COMMIT_EDIT                 800

// Argument Dialog
#define IDC_NAME_EDIT                   800
#define IDC_TYPE_COMBO                  801
//#define IDC_DESCRIPTION_EDIT            802

// Refactor dialog
#define IDC_SYMBOL_LIST                 800
//#define IDC_SYMBOL_EDIT                 801

// Symbol dialog
#define IDC_SYMBOL_EDIT                 802
#define IDC_RENAME_EDIT                 803
#define IDC_PREVIEW_CHECK               804

// Export porject dialog
//#define IDC_FILENAME_EDIT               801
//#define IDC_FOLDER_EDIT                 802
#define IDC_ZIP_RADIO                   803
#define IDC_SPK_RADIO                   804
#define IDC_FOLDER_RADIO                805

///////////////////////////////// STRINGS ///////////////////////////////

// Status bar
#define IDS_CARET_STATUS                1015
#define IDS_STATUS_PANE2                1016

// Toolbar
#define IDS_TOOLBAR_CUSTOMIZE           1018


// Command Group Names
#define IDS_COMMAND_GROUP_ARRAY			          1100
#define IDS_COMMAND_GROUP_CUSTOM                 1101
#define IDS_COMMAND_GROUP_FLEET						 1102
#define IDS_COMMAND_GROUP_FLOW_CONTROL				 1103
#define IDS_COMMAND_GROUP_GAME_ENGINE				 1104
#define IDS_COMMAND_GROUP_GRAPH						 1105 
#define IDS_COMMAND_GROUP_MACRO                  1106
#define IDS_COMMAND_GROUP_MARINE						 1107
#define IDS_COMMAND_GROUP_MATHS						 1108
#define IDS_COMMAND_GROUP_MERCHANT					 1109
#define IDS_COMMAND_GROUP_NON_PLAYER				 1110
#define IDS_COMMAND_GROUP_OBJECT_ACTION	 		 1111
#define IDS_COMMAND_GROUP_OBJECT_PROPERTY	 		 1112
#define IDS_COMMAND_GROUP_PASSENGER			 		 1113
#define IDS_COMMAND_GROUP_PILOT					 	 1114
#define IDS_COMMAND_GROUP_PLAYER						 1115
#define IDS_COMMAND_GROUP_SCRIPT_PROPERTY			 1116
#define IDS_COMMAND_GROUP_SHIP_ACTION				 1117
#define IDS_COMMAND_GROUP_SHIP_PROPERTY			 1118
#define IDS_COMMAND_GROUP_SHIP_TRADE				 1119
#define IDS_COMMAND_GROUP_SHIP_WING					 1120
#define IDS_COMMAND_GROUP_STATION_PROPERTY 		 1121
#define IDS_COMMAND_GROUP_STATION_TRADE			 1122
#define IDS_COMMAND_GROUP_STOCK_EXCHANGE		 	 1123
#define IDS_COMMAND_GROUP_STRING						 1124
#define IDS_COMMAND_GROUP_SYSTEM_PROPERTY			 1125
#define IDS_COMMAND_GROUP_UNIVERSE_DATA			 1126
#define IDS_COMMAND_GROUP_UNIVERSE_PROPERTY		 1127
#define IDS_COMMAND_GROUP_USER_INTERFACE			 1128
#define IDS_COMMAND_GROUP_WAR					       1129
#define IDS_COMMAND_GROUP_WARE_PROPERTY			 1130
#define IDS_COMMAND_GROUP_WEAPON_PROPERTY			 1131
#define IDS_COMMAND_GROUP_HIDDEN		             1132
#define IDS_FIRST_COMMAND_GROUP                  IDS_COMMAND_GROUP_ARRAY

// Maintype names
#define IDS_MAIN_TYPE_BULLET                     1140         
#define IDS_MAIN_TYPE_SECTOR                     1141         
#define IDS_MAIN_TYPE_BACKGROUND                 1142         
#define IDS_MAIN_TYPE_SUN                        1143         
#define IDS_MAIN_TYPE_PLANET                     1144         
#define IDS_MAIN_TYPE_DOCK                       1145         
#define IDS_MAIN_TYPE_FACTORY                    1146         // Factory
#define IDS_MAIN_TYPE_SHIP                       1147         // Ship
#define IDS_MAIN_TYPE_LASER                      1148         // Laser
#define IDS_MAIN_TYPE_SHIELD                     1149         // Shield
#define IDS_MAIN_TYPE_MISSILE                    1150         // Missile
#define IDS_MAIN_TYPE_ENERGY                     1151         // Ware
#define IDS_MAIN_TYPE_NATURAL                    1152         // Ware
#define IDS_MAIN_TYPE_BIO                        1153         // Ware
#define IDS_MAIN_TYPE_FOOD                       1154         // Ware
#define IDS_MAIN_TYPE_MINERAL                    1155         // Ware
#define IDS_MAIN_TYPE_TECH                       1156         // Ware
#define IDS_MAIN_TYPE_ASTEROID                   1157
#define IDS_MAIN_TYPE_GATE                       1158 
#define IDS_MAIN_TYPE_CAMERA                     1159 
#define IDS_MAIN_TYPE_SPECIAL                    1160 
#define IDS_MAIN_TYPE_NEBULA                     1161 
#define IDS_MAIN_TYPE_INSTATION                  1162 
#define IDS_MAIN_TYPE_DUMMY                      1163 
#define IDS_MAIN_TYPE_COMMAND                    1164 
#define IDS_MAIN_TYPE_COCKPIT                    1165 
#define IDS_MAIN_TYPE_DATATYPE                   1166 
#define IDS_MAIN_TYPE_UNKNOWN                    1167 
#define IDS_MAIN_TYPE_DEBRIS                     1168 
#define IDS_MAIN_TYPE_DOCK_WRECK                 1169 
#define IDS_MAIN_TYPE_FACTORY_WRECK              1170 
#define IDS_MAIN_TYPE_SHIP_WRECK                 1171 
#define IDS_FIRST_MAIN_TYPE                      IDS_MAIN_TYPE_BULLET


// ScriptObject Group names
#define IDS_SCRIPT_OBJECT_GROUP_CONSTANT         1180      // Script Constant
#define IDS_SCRIPT_OBJECT_GROUP_DATA_TYPE        1181      // Variable Data Type
#define IDS_SCRIPT_OBJECT_GROUP_FLIGHT_RETURN    1182      // Flight Return Code
#define IDS_SCRIPT_OBJECT_GROUP_OBJECT_CLASS     1183      // Object Class
#define IDS_SCRIPT_OBJECT_GROUP_RACE             1184      // Race
#define IDS_SCRIPT_OBJECT_GROUP_RELATION         1185      // Relation
#define IDS_SCRIPT_OBJECT_GROUP_PARAMETER_TYPE   1186      // Parameter Syntax
#define IDS_SCRIPT_OBJECT_GROUP_SECTOR           1187      // Sector
#define IDS_SCRIPT_OBJECT_GROUP_STATION_SERIAL   1188      // Station Serial
#define IDS_SCRIPT_OBJECT_GROUP_OBJECT_COMMAND   1189      // Object Command
#define IDS_SCRIPT_OBJECT_GROUP_WING_COMMAND     1190      // Wing Command
#define IDS_SCRIPT_OBJECT_GROUP_SIGNAL           1191      // Signal
#define IDS_SCRIPT_OBJECT_GROUP_TRANSPORT_CLASS  1192      // Transport Class
#define IDS_SCRIPT_OBJECT_GROUP_OPERATOR         1193      // Operator
#define IDS_FIRST_SCRIPT_OBJECT_GROUP            IDS_SCRIPT_OBJECT_GROUP_CONSTANT



// Language page group names
#define IDS_LANGUAGE_GROUP_DATA                  1200
#define IDS_LANGUAGE_GROUP_DIALOGUE              1201
#define IDS_LANGUAGE_GROUP_MISC                  1202
#define IDS_LANGUAGE_GROUP_QUEST                 1203
#define IDS_LANGUAGE_GROUP_BONUS                 1204
#define IDS_LANGUAGE_GROUP_MENU                  1205
#define IDS_LANGUAGE_GROUP_EDITOR                1206
#define IDS_LANGUAGE_GROUP_NEWS                  1207
#define IDS_LANGUAGE_GROUP_PLOT                  1208
#define IDS_LANGUAGE_GROUP_NPC                   1209
#define IDS_LANGUAGE_GROUP_USER                  1210
#define IDS_FIRST_LANGUAGE_GROUP                 IDS_LANGUAGE_GROUP_DATA


// Errors
#define ERR_XML_MISSING_ELEMENT         1000
#define ERR_XML_UNEXPECTED_ELEMENT      1001
#define ERR_XML_MISSING_ATTRIBUTE       1002
#define ERR_XML_PARSE_FAILED            1003

#define ERR_LANGUAGE_ID_INVALID         1004
#define ERR_ARGUMENT_NULL               1005
#define ERR_NO_READ_ACCESS              1006
#define ERR_NO_WRITE_ACCESS             1007
#define ERR_NO_SEEK_ACCESS              1008

