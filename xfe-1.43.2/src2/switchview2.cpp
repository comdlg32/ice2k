// Switch between the four possible panel views
long XFileExplorer::onCmdShowPanels(FXObject* sender,FXSelector sel,void* ptr)
{
    // Get window width and height
    int window_width=getWidth();
    int window_height=getHeight();

    switch(FXSELID(sel))
    {
    case ID_SHOW_ONE_PANEL:
        panel_view=ONE_PANEL;
        if (dirpanel->shown())
            dirpanel->handle(sender,FXSEL(SEL_COMMAND,DirPanel::ID_TOGGLE_TREE),ptr);
        if (rpanel->shown())
            rpanel->handle(sender,FXSEL(SEL_COMMAND,FXWindow::ID_TOGGLESHOWN),ptr);
        // Handle drag corner
        rpanel->showCorner(false);
        lpanel->showCorner(true);
        // Handle active icon
        lpanel->showActiveIcon(false);

        break;

    case ID_SHOW_TWO_PANELS:
        panel_view=TWO_PANELS;
        if (vertpanels)
            lpanel->setWidth((int)round(twopanels_lpanel_pct*window_width));
        else
            lpanel->setHeight((int)round(twopanels_lpanel_pct*window_height));
        if (dirpanel->shown())
            dirpanel->handle(sender,FXSEL(SEL_COMMAND,DirPanel::ID_TOGGLE_TREE),ptr);
        if (!rpanel->shown())
            rpanel->handle(sender,FXSEL(SEL_COMMAND,FXWindow::ID_TOGGLESHOWN),ptr);
        // Handle drag corner
        rpanel->showCorner(true);
        lpanel->showCorner(false);
        // Handle active icon
        lpanel->showActiveIcon(true);
        break;

    case ID_SHOW_TREE_PANEL:
        panel_view=TREE_PANEL;
        dirpanel->setWidth((int)round(treepanel_tree_pct*window_width) );
        if (!dirpanel->shown())
            dirpanel->handle(sender,FXSEL(SEL_COMMAND,DirPanel::ID_TOGGLE_TREE),ptr);
        if (rpanel->shown())
            rpanel->handle(sender,FXSEL(SEL_COMMAND,FXWindow::ID_TOGGLESHOWN),ptr);
        // Handle drag corner
        rpanel->showCorner(false);
        lpanel->showCorner(true);
        // Handle active icon
        lpanel->showActiveIcon(true);
        break;

    case ID_SHOW_TREE_TWO_PANELS:
        panel_view=TREE_TWO_PANELS;
        dirpanel->setWidth((int)round(treetwopanels_tree_pct*window_width) );
        if (vertpanels)
            lpanel->setWidth((int)round(treetwopanels_lpanel_pct*window_width));
        else
            lpanel->setHeight((int)round(treetwopanels_lpanel_pct*window_height));
        if (!dirpanel->shown())
            dirpanel->handle(sender,FXSEL(SEL_COMMAND,DirPanel::ID_TOGGLE_TREE),ptr);
        if (!rpanel->shown())
            rpanel->handle(sender,FXSEL(SEL_COMMAND,FXWindow::ID_TOGGLESHOWN),ptr);
        // Handle drag corner
        lpanel->showCorner(false);
        rpanel->showCorner(true);
        // Handle active icon
        lpanel->showActiveIcon(true);
        break;
    }

    // Set focus on current panel
    lpanel->getCurrent()->setFocusOnList();

    return 1;
}
