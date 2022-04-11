#ifndef GUISANDBOX_H
#define GUISANDBOX_H

/* This generated file contains includes for project dependencies */
#include "bake_config.h"
#include "UI.h"
#include <gtk/gtk.h>
#include <string>

class GUISandbox : public UI::Application
{
 public:
    GUISandbox();
    ~GUISandbox();
    
    void OnStartup() override;
    void OnUpdate() override;
    void OnShutdown() override;
    
    static void SetUpLowerButtonBox(GtkWidget* buttonBox);
    static GtkWidget* InitializeWindow(GtkApplication* app, const char* title);
    static GtkWidget* InitializeButtonBox(GtkWidget* window, GtkOrientation orientation);
    static GtkWidget* InitializeCloseButton(GtkWindow* window);
    static GtkWidget* InitializeStartButton(GtkWindow* window);
    static GtkWidget* InitializeRestartButton(GtkWindow* window);
    static GtkWidget* InitializeStopButton(GtkWindow* window);
    static GtkWidget* InitializeStatsButton(GtkWindow* window); 
    static GtkWidget* InitializeDataButton(GtkWindow* window);
    static GtkWidget* InitializeGeneralRulesButton(GtkWindow* window);
    static GtkWidget* InitializeSpecificRulesButton(GtkWindow* window);
    static GtkWidget* InitializeRulesButton(GtkWindow* window);
    static GtkWidget* InitializeShowRulesButton(GtkWindow* window);
    static GtkWidget* InitializeShowUrlsButton(GtkWindow* window);
    static GtkWidget* InitializeAddRuleButton(GtkWindow* window);
    static GtkWidget* InitializeRemoveRuleButton(GtkWindow* window);
    static GtkWidget* InitializeAddUrlButton(GtkWindow* window);
    static GtkWidget* InitializeRemoveUrlButton(GtkWindow* window);
    static GtkWidget* InitializeUrlAdderButton(GtkWindow* window, GtkWidget* entry);
    
    static GtkWidget* InitializeSpinButton(const char* name, double rate);
    
    static void PrintRulesRunHandle();
    static void PrintUrlsRunHandle();
    
    static void SetUpMainWindow(GtkApplication* app);
    static void SetUpStatsWindow();
    static void SetUpRulesWindow();
    static void SetUpAddUrlWindow();
    static void SetUpShowRulesWindow();
    static void SetUpShowUrlsWindow();
    
    static void UpdateCommand(std::string command);
    static void OnAppActivated(GtkApplication* app, gpointer userData);
    static void OnAppClosed(GtkApplication* app, gpointer userData);
    static void OnCloseButtonClicked(GtkWidget* button, gpointer userData);
    static void OnStartButtonClicked(GtkWidget* button, gpointer userData);
    static void OnRestartButtonClicked(GtkWidget* button, gpointer userData);
    static void OnStopButtonClicked(GtkWidget* button, gpointer userData);
    static void OnStatsButtonClicked(GtkWidget* button, gpointer userData);
    static void OnDataButtonClicked(GtkWidget* button, gpointer userData);
    static void OnGeneralRulesButtonClicked(GtkWidget* button, gpointer userData);
    static void OnSpecificRulesButtonClicked(GtkWidget* button, gpointer userData);
    static void OnMessageDialogResponse(GtkDialog* dialog, gint responseId, gpointer userData);
    
    static void OnCloseWindowClicked(GtkApplication* app, gpointer userData);
    static void OnCloseStatsWindowClicked(GtkApplication* app, gpointer userData);
    static void OnCloseRulesWindowClicked(GtkApplication* app, gpointer userData);
    static void OnCloseSubRuleWindowClicked(GtkWidget* window);
    
    static void OnRulesButtonClicked(GtkWidget* button, gpointer userData);
    static void OnShowRulesButtonClicked(GtkWidget* button, gpointer userData);
    static void OnShowUrlsButtonClicked(GtkWidget* button, gpointer userData);
    static void OnAddRuleButtonClicked(GtkWidget* button, gpointer userData);
    static void OnRemoveRuleButtonClicked(GtkWidget* button, gpointer userData);
    static void OnAddUrlButtonClicked(GtkWidget* button, gpointer userData);
    static void OnRemoveUrlButtonClicked(GtkWidget* button, gpointer userData);
    
    static void OnUrlAdderButtonClicked(GtkWidget* button, gpointer userData);
    
    static void CreateAndDisplayMessageDialog(GtkWindow* window, const char * message, GtkMessageType type);
    static GtkWidget* CreateMessageDialog(GtkWindow* window, const char* message, GtkMessageType type);
        
    static void minIdValueChanged(GtkWidget* spin, gpointer userData);
    static void maxIdValueChanged(GtkWidget* spin, gpointer userData);
    
    static void ParseByChar(std::string& data, char divider, std::vector<std::string>& vec);

    void ShutDownGUI();
    void RunGUI();
    int StartGUI();
 
 private:
    static std::string _commandRes;
    
    static GtkApplication* _application;
    static bool _isUpdated;
    static GtkWidget* _mainWindow;
    static GtkWidget* _statsWindow;
    static GtkWidget* _rulesWindow;
    static GtkWidget* _addUrlWindow;
    static GtkWidget* _ruleViewerWindow;
    static GtkWidget* _urlViewerWindow;
    
    static GtkWidget* _urlListBox;
    static GtkWidget* _ruleListBox;
    
    static bool _addUrlWasClicked;
    static bool _statsWasClicked;
    static bool _rulesWasClicked;
    static bool _ruleShowWasClicked;
    static bool _urlShowWasClicked;
    static bool _refreshRules;
    static bool _refreshUrls;

    static bool _waitForUrlAddding;
    static bool _waitForShowRules;
    static bool _waitForShowUrls;
    
    static int _minRuleID;
    static int _maxRuleID;
    
    GMainContext* _context;
};


#endif

