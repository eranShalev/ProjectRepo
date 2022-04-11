#include "GUISandbox.h"

#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <string>

std::string GUISandbox::_commandRes = "";
GtkApplication* GUISandbox::_application;
GtkWidget* GUISandbox::_mainWindow;
GtkWidget* GUISandbox::_statsWindow;
GtkWidget* GUISandbox::_rulesWindow;
GtkWidget* GUISandbox::_addUrlWindow;
GtkWidget* GUISandbox::_ruleViewerWindow;
GtkWidget* GUISandbox::_urlViewerWindow;

GtkWidget* GUISandbox::_ruleListBox;
GtkWidget* GUISandbox::_urlListBox;

bool GUISandbox::_isUpdated = false;

bool GUISandbox::_statsWasClicked = false;
bool GUISandbox::_rulesWasClicked = false;
bool GUISandbox::_addUrlWasClicked = false;
bool GUISandbox::_ruleShowWasClicked = false;
bool GUISandbox::_urlShowWasClicked = false;

bool GUISandbox::_waitForUrlAddding = false;
bool GUISandbox::_waitForShowRules = false;
bool GUISandbox::_waitForShowUrls = false;

bool GUISandbox::_refreshRules = false;
bool GUISandbox::_refreshUrls = false;

int GUISandbox::_minRuleID = 1;
int GUISandbox::_maxRuleID = 1;

GUISandbox::GUISandbox()
{
}

GUISandbox::~GUISandbox()
{
}

void GUISandbox::ParseByChar(std::string& data, char divider, std::vector<std::string>& vec)
{
    std::string substring;
    std::stringstream data_stream(data);
    
    while(std::getline(data_stream, substring, divider))
    {
        vec.push_back(substring);
    }
}

void GUISandbox::OnStartup()
{
    _isGUI = true;
    _isUpdated = false;
    _currCommand = "none";
    StartGUI();
    UI::Application::OnStartup();
}

void GUISandbox::OnUpdate()
{
    RunGUI();
    
    if (_isUpdated)
    {
        _commandRes = OnUpdateImpl();
        std::cout << _commandRes << std::endl;
        _isUpdated = false;
    }
}

void GUISandbox::OnShutdown()
{
    UI::Application::OnShutdown();
    ShutDownGUI();
}

void GUISandbox::ShutDownGUI()
{
    while (g_main_context_iteration(_context, false));
    
    g_main_context_release(_context);
    g_object_unref(_application);
}

void GUISandbox::RunGUI()
{
    if (_waitForUrlAddding)
    {
        _waitForUrlAddding = false;
        if (_commandRes == "no url was added")
        {
            CreateAndDisplayMessageDialog(GTK_WINDOW(_addUrlWindow), "Error: Duplicate Urls", GTK_MESSAGE_WARNING);
        }
        else
        {
            CreateAndDisplayMessageDialog(GTK_WINDOW(_addUrlWindow), _commandRes.c_str(), GTK_MESSAGE_INFO);
        }
    }
    else if (_waitForShowUrls)
    {
        PrintUrlsRunHandle();
    }
    else if (_waitForShowRules)
    {
        PrintRulesRunHandle();
    }   
    else if (_refreshRules)
    {
        _refreshRules = false;
        UpdateCommand("print");
        _waitForShowRules = true;
    }
    else if (_refreshUrls)
    {
        _refreshUrls = false;
        UpdateCommand("print url");
        _waitForShowUrls = true;
    }

    do
    {
        while (!g_main_context_iteration(_context, true));
    } while (g_main_context_pending(_context));

}

// TODO(eran): Remove not used methods args
int GUISandbox::StartGUI()
{
    _application = gtk_application_new("com.eran-roei.gtktest", G_APPLICATION_FLAGS_NONE);
    _context = g_main_context_default();
    
    if (!g_main_context_acquire(_context))
    {
        g_print("couldnt acquire context\n");
        g_object_unref(_application);
        return 1;
    }
    
    if (!g_application_register(G_APPLICATION(_application), nullptr, NULL))
    {
        g_print("couldnt register application\n");
        g_object_unref(_application);
        return 1;
    }
    
    //Starting Application Activation
    g_signal_connect(_application, "activate", G_CALLBACK(OnAppActivated), nullptr);
    g_application_activate(G_APPLICATION(_application));
    
    return 0;
}

void GUISandbox::PrintRulesRunHandle()
{
    _waitForShowRules = false;
    
    //gtk_container_foreach(GTK_CONTAINER(_ruleListBox), G_CALLBACK(ClearChilds), nullptr);
    
    GList *children, *iter;

    children = gtk_container_get_children(GTK_CONTAINER(_ruleListBox));
    
    for(iter = children; iter != NULL; iter = g_list_next(iter))
    {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    
    g_list_free(children);
        
    std::vector<std::string> rulesFromUI;
    ParseByChar(_commandRes, '\n', rulesFromUI);
    rulesFromUI.erase(rulesFromUI.begin());
        
    if (rulesFromUI.size() > 0)
    {
        for (std::string ruleFromUI : rulesFromUI)
        {
            GtkWidget* ruleLabel = gtk_label_new(ruleFromUI.c_str());
            gtk_list_box_insert(GTK_LIST_BOX(_ruleListBox), ruleLabel, -1);
        }
            
        gtk_widget_show_all(_ruleViewerWindow);
    }
}

void GUISandbox::PrintUrlsRunHandle()
{
    _waitForShowUrls = false;
        
    GList *children, *iter;

    children = gtk_container_get_children(GTK_CONTAINER(_urlListBox));
    
    for(iter = children; iter != NULL; iter = g_list_next(iter))
    {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    
    g_list_free(children);
        
    std::vector<std::string> urlsFromUI;
    ParseByChar(_commandRes, '\n', urlsFromUI);
    //urlsFromUI.erase(urlsFromUI.begin());
        
    if (urlsFromUI.size() > 0)
    {
        for (std::string urlFromUI : urlsFromUI)
        {
            GtkWidget* ruleLabel = gtk_label_new(urlFromUI.c_str());
            gtk_list_box_insert(GTK_LIST_BOX(_urlListBox), ruleLabel, -1);
        }
            
        gtk_widget_show_all(_urlViewerWindow);
    }
    
}

void GUISandbox::SetUpMainWindow(GtkApplication* app)
{
    _mainWindow = InitializeWindow(app, "Main Window");
    
    GtkWidget* bottomButtonBox = InitializeButtonBox(_mainWindow, GTK_ORIENTATION_HORIZONTAL);
    
    GtkWidget* closeButton = InitializeCloseButton(GTK_WINDOW(_mainWindow));
    GtkWidget* startButton = InitializeStartButton(GTK_WINDOW(_mainWindow));
    GtkWidget* restartButton = InitializeRestartButton(GTK_WINDOW(_mainWindow));
    GtkWidget* stopButton = InitializeStopButton(GTK_WINDOW(_mainWindow));
    GtkWidget* statsButton = InitializeStatsButton(GTK_WINDOW(_mainWindow));
    GtkWidget* rulesButton = InitializeRulesButton(GTK_WINDOW(_mainWindow));
    
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), startButton);
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), statsButton);
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), rulesButton);
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), restartButton);
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), stopButton);
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), closeButton);
    
    g_signal_connect (_mainWindow, "delete-event", G_CALLBACK(OnCloseWindowClicked), NULL);
    gtk_widget_show_all(_mainWindow);
}

void GUISandbox::SetUpStatsWindow()
{
    _statsWindow = InitializeWindow(_application, "stats window");
    
    _statsWasClicked = true;
    
    GtkWidget* bottomButtonBox = InitializeButtonBox(
                                 _statsWindow,
                                 GTK_ORIENTATION_HORIZONTAL);
    
    GtkWidget* dataButton = InitializeDataButton(
                             GTK_WINDOW(_statsWindow));
    
    GtkWidget* generalRulesButton = InitializeGeneralRulesButton(
                             GTK_WINDOW(_statsWindow));
    
    GtkWidget* specificRulesButton = InitializeSpecificRulesButton(
                               GTK_WINDOW(_statsWindow));
    
    GtkWidget* minIdSpinnerButton = InitializeSpinButton("Min Rule Id", 1);
    GtkWidget* maxIdSpinnerButton = InitializeSpinButton("Max Rule Id", 1);
    
    g_signal_connect(minIdSpinnerButton, "value-changed", G_CALLBACK(minIdValueChanged), NULL);
    g_signal_connect(maxIdSpinnerButton, "value-changed", G_CALLBACK(maxIdValueChanged), NULL);
    
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), dataButton);
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), generalRulesButton);
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), specificRulesButton);
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), minIdSpinnerButton);
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), maxIdSpinnerButton);
    
    g_signal_connect (_statsWindow, "delete-event", G_CALLBACK(OnCloseStatsWindowClicked), NULL);
}

void GUISandbox::SetUpRulesWindow()
{
    _rulesWindow = InitializeWindow(_application, "rules window");
    
    _rulesWasClicked = true;
    
    GtkWidget* bottomButtonBox = InitializeButtonBox(
                                 _rulesWindow,
                                 GTK_ORIENTATION_HORIZONTAL);
    
    GtkWidget* showRulesButton = InitializeShowRulesButton(
                             GTK_WINDOW(_rulesWindow));
    
    GtkWidget* showUrlsButton = InitializeShowUrlsButton(
                             GTK_WINDOW(_rulesWindow));
    
    GtkWidget* addRuleButton = InitializeAddRuleButton(
                               GTK_WINDOW(_rulesWindow));
    
    GtkWidget* addUrlButton = InitializeAddUrlButton(
                               GTK_WINDOW(_rulesWindow));
    
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), showRulesButton);
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), showUrlsButton);
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), addRuleButton);
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), addUrlButton);
    
    g_signal_connect (_rulesWindow, "delete-event", G_CALLBACK(OnCloseRulesWindowClicked), NULL);
}

void GUISandbox::SetUpAddUrlWindow()
{
    _addUrlWindow = InitializeWindow(_application, "Url Adder");
    
    _addUrlWasClicked = true;
    
    GtkWidget* bottomButtonBox = InitializeButtonBox(
                                 _addUrlWindow,
                                 GTK_ORIENTATION_VERTICAL);
    
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bottomButtonBox), GTK_BUTTONBOX_CENTER);
    GtkWidget* urlEntry = gtk_entry_new();
    
    gtk_entry_set_text(GTK_ENTRY(urlEntry), "www.example.com");
    
    GtkWidget* addUrlButton = InitializeUrlAdderButton(GTK_WINDOW(_addUrlWindow), urlEntry);
    
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), urlEntry);
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), addUrlButton);
    
    g_signal_connect(_addUrlWindow, "delete-event",
                     G_CALLBACK(OnCloseSubRuleWindowClicked),
                     _addUrlWindow);
}

void GUISandbox::SetUpShowRulesWindow()
{
    _ruleViewerWindow = InitializeWindow(_application, "Rules Viewer");
    _ruleShowWasClicked = true;
    
    GtkWidget* bottomButtonBox = InitializeButtonBox(
                                 _ruleViewerWindow,
                                 GTK_ORIENTATION_HORIZONTAL);
    
    GtkWidget* removeRuleButton = InitializeRemoveRuleButton(
                               GTK_WINDOW(_ruleViewerWindow));
    
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bottomButtonBox), GTK_BUTTONBOX_CENTER);
    
    _ruleListBox = gtk_list_box_new();
    
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), _ruleListBox);
    gtk_container_add(GTK_CONTAINER(_ruleViewerWindow), bottomButtonBox);
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), removeRuleButton);
    
    g_signal_connect(_ruleViewerWindow, "delete-event",
                     G_CALLBACK(OnCloseSubRuleWindowClicked),
                     _ruleViewerWindow);
}

void GUISandbox::SetUpShowUrlsWindow()
{
    _urlViewerWindow = InitializeWindow(_application, "URL Viewer");
    
    _urlShowWasClicked = true;
    
    GtkWidget* bottomButtonBox = InitializeButtonBox(
                                 _urlViewerWindow,
                                 GTK_ORIENTATION_HORIZONTAL);
    
    GtkWidget* removeUrlButton = InitializeRemoveUrlButton(
                               GTK_WINDOW(_urlViewerWindow));
    
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bottomButtonBox), GTK_BUTTONBOX_CENTER);
    
    _urlListBox = gtk_list_box_new();
    
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), _urlListBox);
    gtk_container_add(GTK_CONTAINER(_urlViewerWindow), bottomButtonBox);
    gtk_container_add(GTK_CONTAINER(bottomButtonBox), removeUrlButton);
    
    g_signal_connect(_urlViewerWindow, "delete-event",
                     G_CALLBACK(OnCloseSubRuleWindowClicked),
                     _urlViewerWindow);
}

GtkWidget* GUISandbox::InitializeSpinButton(const char* name, double rate)
{
    GtkAdjustment* adjustment = gtk_adjustment_new(1, 1, 10000, 1, 10, 0);
    GtkWidget* spinButton = gtk_spin_button_new(adjustment, rate, 1);
    
    return spinButton;
}

void GUISandbox::SetUpLowerButtonBox(GtkWidget* buttonBox)
{
    gtk_button_box_set_layout(GTK_BUTTON_BOX(buttonBox), GTK_BUTTONBOX_EDGE);
}

GtkWidget* GUISandbox::InitializeWindow(GtkApplication* app, const char* title)
{
    GtkWidget* window = gtk_application_window_new(app);
    
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    
    return window;
}

GtkWidget* GUISandbox::InitializeButtonBox(GtkWidget* window, GtkOrientation orientation)
{
    GtkWidget* buttonBox = gtk_button_box_new(orientation);
    gtk_container_add(GTK_CONTAINER(window), buttonBox);
    return buttonBox;
}

GtkWidget* GUISandbox::InitializeUrlAdderButton(GtkWindow* window, GtkWidget* entry)
{
    GtkWidget* urlAdder = gtk_button_new_with_label("confirm");
    
    g_signal_connect(urlAdder,
                     "clicked",
                     G_CALLBACK(OnUrlAdderButtonClicked), entry);

    return urlAdder;
}

GtkWidget* GUISandbox::InitializeCloseButton(GtkWindow* window)
{
    GtkWidget* closeButton = gtk_button_new_with_label("close");
    g_signal_connect(closeButton,
                     "clicked",
                     G_CALLBACK(OnCloseButtonClicked), window);
    
    return closeButton;
}

GtkWidget* GUISandbox::InitializeStartButton(GtkWindow* window)
{
    GtkWidget* startButton = gtk_button_new_with_label("start");
    g_signal_connect(startButton,
                     "clicked",
                     G_CALLBACK(OnStartButtonClicked), window);
    
    return startButton;
}

GtkWidget* GUISandbox::InitializeStatsButton(GtkWindow* window)
{
    GtkWidget* statsButton = gtk_button_new_with_label("stats");
    g_signal_connect(statsButton,
                     "clicked",
                     G_CALLBACK(OnStatsButtonClicked), window);
    
    return statsButton;
}

GtkWidget* GUISandbox::InitializeRulesButton(GtkWindow* window)
{
    GtkWidget* rulesButton = gtk_button_new_with_label("rules");
    g_signal_connect(rulesButton,
                     "clicked",
                     G_CALLBACK(OnRulesButtonClicked), window);
    
    return rulesButton;
}

GtkWidget* GUISandbox::InitializeRestartButton(GtkWindow* window)
{
    GtkWidget* restartButton = gtk_button_new_with_label("restart");
    g_signal_connect(restartButton,
                     "clicked",
                     G_CALLBACK(OnRestartButtonClicked), window);
    return restartButton;
}

GtkWidget* GUISandbox::InitializeStopButton(GtkWindow* window)
{
    GtkWidget* stopButton = gtk_button_new_with_label("stop");
    g_signal_connect(stopButton,
                     "clicked",
                     G_CALLBACK(OnStopButtonClicked), window);
    return stopButton;
}

GtkWidget* GUISandbox::InitializeDataButton(GtkWindow* window)
{
    GtkWidget* dataButton = gtk_button_new_with_label("data");
    g_signal_connect(dataButton,
                     "clicked",
                     G_CALLBACK(OnDataButtonClicked), window);
    return dataButton;
}

GtkWidget* GUISandbox::InitializeGeneralRulesButton(GtkWindow* window)
{
    GtkWidget* generalButton = gtk_button_new_with_label("general rules");
    g_signal_connect(generalButton,
                     "clicked",
                     G_CALLBACK(OnGeneralRulesButtonClicked), window);
    return generalButton;
}

GtkWidget* GUISandbox::InitializeSpecificRulesButton(GtkWindow* window)
{
    GtkWidget* specificButton = gtk_button_new_with_label("specific rules");
    g_signal_connect(specificButton,
                     "clicked",
                     G_CALLBACK(OnSpecificRulesButtonClicked), window);
    return specificButton;
}

GtkWidget* GUISandbox::InitializeShowRulesButton(GtkWindow* window)
{
    GtkWidget* showRulesButton = gtk_button_new_with_label("Show Rules");
    g_signal_connect(showRulesButton,
                     "clicked",
                     G_CALLBACK(OnShowRulesButtonClicked), window);
    return showRulesButton;
}

GtkWidget* GUISandbox::InitializeShowUrlsButton(GtkWindow* window)
{
    GtkWidget* showUrlsButton = gtk_button_new_with_label("Show Urls");
    g_signal_connect(showUrlsButton,
                     "clicked",
                     G_CALLBACK(OnShowUrlsButtonClicked), window);
    return showUrlsButton;
}

GtkWidget* GUISandbox::InitializeAddRuleButton(GtkWindow* window)
{
    GtkWidget* addRuleButton = gtk_button_new_with_label("Add Rule");
    g_signal_connect(addRuleButton,
                     "clicked",
                     G_CALLBACK(OnAddRuleButtonClicked), window);
    return addRuleButton;
}

GtkWidget* GUISandbox::InitializeRemoveRuleButton(GtkWindow* window)
{
    GtkWidget* removeRuleButton = gtk_button_new_with_label("Remove Rule");
    g_signal_connect(removeRuleButton,
                     "clicked",
                     G_CALLBACK(OnRemoveRuleButtonClicked), window);
    return removeRuleButton;
}

GtkWidget* GUISandbox::InitializeAddUrlButton(GtkWindow* window)
{
    GtkWidget* addUrlButton = gtk_button_new_with_label("Add Url");
    g_signal_connect(addUrlButton,
                     "clicked",
                     G_CALLBACK(OnAddUrlButtonClicked), window);
    return addUrlButton;
}

GtkWidget* GUISandbox::InitializeRemoveUrlButton(GtkWindow* window)
{
    GtkWidget* removeUrlButton = gtk_button_new_with_label("Remove Url");
    g_signal_connect(removeUrlButton,
                     "clicked",
                     G_CALLBACK(OnRemoveUrlButtonClicked), window);
    return removeUrlButton;
}

void GUISandbox::OnAppActivated(GtkApplication* app, gpointer userData)
{
    g_print("activating application\n");
    SetUpMainWindow(_application);
}

void GUISandbox::UpdateCommand(std::string command)
{
    _currCommand = command;
    _isUpdated = true;
}

GtkWidget* GUISandbox::CreateMessageDialog(GtkWindow* window, const char* message, GtkMessageType type)
{
    GtkWidget* messageDialog = gtk_message_dialog_new(
                                                      window,
                                                      GTK_DIALOG_MODAL,
                                                      type,
                                                      GTK_BUTTONS_OK,
                                                      message);
    return messageDialog;
}

void GUISandbox::OnCloseButtonClicked(GtkWidget* button, gpointer userData)
{
    UpdateCommand("close");
    
    GtkWindow* window = GTK_WINDOW(userData);
    
    CreateAndDisplayMessageDialog(window, "Firewall Shutting Down!", GTK_MESSAGE_INFO);
}

void GUISandbox::OnStartButtonClicked(GtkWidget* button, gpointer userData)
{
    UpdateCommand("start");
    
    GtkWindow* window = GTK_WINDOW(userData);
    
    CreateAndDisplayMessageDialog(window, "Starting Firwall!", GTK_MESSAGE_INFO);
    
}

void GUISandbox::OnRestartButtonClicked(GtkWidget* button, gpointer userData)
{
    UpdateCommand("restart");
    
    GtkWindow* window = GTK_WINDOW(userData);
    
    CreateAndDisplayMessageDialog(window, "Firewall Restarting!", GTK_MESSAGE_INFO);
}

void GUISandbox::OnStopButtonClicked(GtkWidget* button, gpointer userData)
{
    UpdateCommand("stop");
    
    GtkWindow* window = GTK_WINDOW(userData);
    
    CreateAndDisplayMessageDialog(window, "Firewall Stopped Filtering!", GTK_MESSAGE_INFO);
}

void GUISandbox::OnDataButtonClicked(GtkWidget* button, gpointer userData)
{
    UpdateCommand("data");
}

void GUISandbox::OnGeneralRulesButtonClicked(GtkWidget* button, gpointer userData)
{
    UpdateCommand("general");
}

void GUISandbox::OnSpecificRulesButtonClicked(GtkWidget* button, gpointer userData)
{
    UpdateCommand("rule " + std::to_string(_minRuleID) + " " + std::to_string(_maxRuleID));
}

void GUISandbox::OnStatsButtonClicked(GtkWidget* button, gpointer userData)
{
    UpdateCommand("stats");
    
    gtk_widget_hide(_mainWindow);
    
    if (!_statsWasClicked)
    {
        _statsWindow = InitializeWindow(_application, "stats window");
        g_signal_connect (_statsWindow, "delete-event", G_CALLBACK(OnCloseStatsWindowClicked), NULL);
        
        SetUpStatsWindow();
    }
    
    gtk_widget_show_all(_statsWindow);
}

void GUISandbox::OnShowRulesButtonClicked(GtkWidget* button, gpointer userData)
{
    UpdateCommand("print");
    
    gtk_widget_hide(_rulesWindow);
    
    if (!_ruleShowWasClicked)
    {
        SetUpShowRulesWindow();
    }
    
    _waitForShowRules = true;
    
    gtk_widget_show_all(_ruleViewerWindow);
}

void GUISandbox::OnShowUrlsButtonClicked(GtkWidget* button, gpointer userData)
{
    UpdateCommand("print url");
    
    gtk_widget_hide(_rulesWindow);
    
    if (!_urlShowWasClicked)
    {
        SetUpShowUrlsWindow();
    }
    
    _waitForShowUrls = true;
    
    gtk_widget_show_all(_urlViewerWindow);
}

void GUISandbox::OnAddRuleButtonClicked(GtkWidget* button, gpointer userData)
{
    
}

void GUISandbox::OnRemoveRuleButtonClicked(GtkWidget* button, gpointer userData)
{
    GtkListBoxRow* selectedRow = gtk_list_box_get_selected_row(GTK_LIST_BOX(_ruleListBox));
    
    if (selectedRow != NULL)
    {
        GtkWidget* labelChild = gtk_bin_get_child(GTK_BIN(selectedRow));
        std::string commandMsg = gtk_label_get_text(GTK_LABEL(labelChild));
        commandMsg = "remove " + commandMsg.substr(0, commandMsg.find(" "));
        UpdateCommand(commandMsg);
        _refreshRules = true;
    }
    else
    {
        std::cout << "no row was selected" << std::endl;
    } 
}

void GUISandbox::OnAddUrlButtonClicked(GtkWidget* button, gpointer userData)
{
    gtk_widget_hide(_rulesWindow);
    
    if (!_addUrlWasClicked)
    {
        SetUpAddUrlWindow();
    }
    
    gtk_widget_show_all(_addUrlWindow);
}

void GUISandbox::OnRemoveUrlButtonClicked(GtkWidget* button, gpointer userData)
{
    GtkListBoxRow* selectedRow = gtk_list_box_get_selected_row(GTK_LIST_BOX(_urlListBox));
    
    if (selectedRow != NULL)
    {
        GtkWidget* labelChild = gtk_bin_get_child(GTK_BIN(selectedRow));
        std::string commandMsg = gtk_label_get_text(GTK_LABEL(labelChild));
        commandMsg = "remove url " + commandMsg.substr(0, commandMsg.find(":"));
        UpdateCommand(commandMsg);
        _refreshUrls = true;
    }
    else
    {
        std::cout << "no row was selected" << std::endl;
    } 
}

void GUISandbox::OnRulesButtonClicked(GtkWidget* button, gpointer userData)
{
    UpdateCommand("rules");
    
    gtk_widget_hide(_mainWindow);
    
    if (!_rulesWasClicked)
    {
        SetUpRulesWindow();
    }
    
    gtk_widget_show_all(_rulesWindow);
}

void GUISandbox::OnUrlAdderButtonClicked(GtkWidget* button, gpointer userData)
{
    GtkWidget* entry = GTK_WIDGET(userData);
   
    std::string commandMsg = gtk_entry_get_text(GTK_ENTRY(entry));
    
    _waitForUrlAddding = true;
    
    UpdateCommand("add url " + commandMsg);
}

void GUISandbox::minIdValueChanged(GtkWidget* spin, gpointer userData)
{
    _minRuleID =  gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
}

void GUISandbox::maxIdValueChanged(GtkWidget* spin, gpointer userData)
{
    _maxRuleID = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
}

void GUISandbox::OnMessageDialogResponse(GtkDialog* dialog, gint responseId, gpointer userData)
{
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

void GUISandbox::OnAppClosed(GtkApplication* app, gpointer userData)
{
    g_object_unref(app);
}

void GUISandbox::OnCloseWindowClicked(GtkApplication* app, gpointer userData)
{
    UpdateCommand("close");
}

void GUISandbox::OnCloseStatsWindowClicked(GtkApplication* app, gpointer userData)
{
    UpdateCommand("exit");
    gtk_widget_show_all(_mainWindow);
    gtk_widget_hide(_statsWindow);
}

void GUISandbox::OnCloseRulesWindowClicked(GtkApplication* app, gpointer userData)
{
    UpdateCommand("exit");
    gtk_widget_show_all(_mainWindow);
    gtk_widget_hide(_rulesWindow);
}

void GUISandbox::OnCloseSubRuleWindowClicked(GtkWidget* window)
{
    gtk_widget_show_all(_rulesWindow);
    gtk_widget_hide(window);
}

void GUISandbox::CreateAndDisplayMessageDialog(GtkWindow* window, const char * message, GtkMessageType type)
{
    GtkWidget* messageDialog = CreateMessageDialog(GTK_WINDOW(window), message, type);
    
    g_signal_connect(GTK_DIALOG(messageDialog),
                     "response",
                     G_CALLBACK(OnMessageDialogResponse),
                     nullptr);
    
    gtk_widget_show_all(messageDialog);
}

UI::Application* UI::CreateApplication()
{
    return new GUISandbox();
}

