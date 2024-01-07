// EasyMon Program.


#include "easymon.h"

// Timeout identifier.
guint timeout_ID;

// Total installed system memory in gigabytes, to be determined on app startup.
double mem_totalInstalled;

// Variables used to display memory values in either gigabytes or as a percentage. 
double memVal_factor;
char memVal_format[MEM_CHAR];
char unit[3];

// Indicator for completion of startup tasks.
bool Ready = false;

/* Indicators for whether or not either expander section was expanded at the
   start of the present timeout interval. These are updated once per interval. */
gboolean Expanded_cpu = FALSE;
gboolean Expanded_mem = FALSE;

// Widget structures from helpers.h.
Main_Container primaryBox = {0};
CPU_Display cpuBox = {0};
Mem_Display memBox = {0};

// CSS provider for color scheme-specific styles.
GtkCssProvider *cssProvider_color;

// Markup template for expander titles. Markup may be modified to adjust title appearance.
const char *titleTemplate =
    "<span foreground=\"#2aa200\" size=\"large\" underline=\"single\">"
        "%s        "
    "</span>"
    "<span foreground=\"#30cdff\">"
        "<span size=\"x-large\">"
            " %s"
        "</span>"
        "<span size=\"small\">"
            " %s "
        "</span>"
    "</span>";

// Memory cleanup; triggered when user exits application.
static void cleanup()
{
    if (cssProvider_color != NULL)
    {
        g_object_unref(cssProvider_color);
    }
}

int main(int argc, char **argv)
{  
    // Create easymon application with identifying string.
    GtkApplication *easymon = gtk_application_new("com.gmail.ctbishop34.EasyMon2", G_APPLICATION_FLAGS_NONE);

    // Set application launch to trigger setup_make_gui function.
    g_signal_connect(easymon, "activate", G_CALLBACK(setup_make_gui), NULL);

    // Launch application, then wait until it is closed.
    int status = g_application_run(G_APPLICATION(easymon), argc, argv);

    // Run cleanup function.
    atexit(cleanup);

    // Free easymon from memory.
    g_object_unref(easymon);

    return status;
}

// Function performs one-time startup tasks.
void setup_make_gui(GtkApplication *easymon, gpointer user_data)
{
    // Create builder for user interface file so that UI components can be accessed.
    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "/usr/share/easymon/ui/easymon.ui", NULL);

    // Get application window from builder, and connect the window to the application.
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    gtk_window_set_application(GTK_WINDOW(window), easymon);

    /* Create CSS provider and use to apply general styles to window,
       then free as it will not be needed again. */
    GtkCssProvider *cssProvider_primary = gtk_css_provider_new();
    css_add_provider_with_styles
    (
        cssProvider_primary,
        "/usr/share/easymon/css/easymon.css",
        window
    );
    g_object_unref(cssProvider_primary);

    // Create CSS provider for color schemes. Use to apply default light scheme to window.
    cssProvider_color = gtk_css_provider_new();
    css_add_provider_with_styles
    (
        cssProvider_color,
        "/usr/share/easymon/css/easymon-light-scheme.css",
        window
    );

    // Get error message and visible window contents so that visibility can be swapped if needed.
    primaryBox.errorMessage = GTK_WIDGET(gtk_builder_get_object(builder, "errorMessage"));
    primaryBox.stackButtons = GTK_WIDGET(gtk_builder_get_object(builder, "stackButtons"));
    primaryBox.stack = GTK_WIDGET(gtk_builder_get_object(builder, "stack"));

    /* Get objects needed to update transient components of cpu display and set handler
       function for user action on cpu detail expander. Function will be triggered when
       user opens or closes expander (i.e., by change in property "expanded"). */
    cpuBox.expander = GTK_EXPANDER(gtk_builder_get_object(builder, "cpuExpander"));
    cpuBox.grid = GTK_GRID(gtk_builder_get_object(builder, "cpuGrid"));
    g_signal_connect
    (
        cpuBox.expander,
        "notify::expanded",
        G_CALLBACK(on_expander_notify_expanded),
        NULL
    );

    /* Repeat the above for objects needed to update transient components of memory display,
       connecting memory expander to the same handler as cpu expander. */
    memBox.totalInstalled = GTK_LABEL(gtk_builder_get_object(builder, "totalInstalled"));
    memBox.expander = GTK_EXPANDER(gtk_builder_get_object(builder, "memoryExpander"));
    memBox.buffers = GTK_LABEL(gtk_builder_get_object(builder, "buffers"));
    memBox.pageCache = GTK_LABEL(gtk_builder_get_object(builder, "pageCache"));
    memBox.swapCache = GTK_LABEL(gtk_builder_get_object(builder, "swapCache"));
    g_signal_connect
    (
        memBox.expander,
        "notify::expanded",
        G_CALLBACK(on_expander_notify_expanded),
        NULL);

    // Get memory units switch and set handler. Call handler to set default units.
    GtkSwitch *unitsSwitch = GTK_SWITCH(gtk_builder_get_object(builder, "unitsSwitch"));
    g_signal_connect
    (
        unitsSwitch,
        "state-set",
        G_CALLBACK(on_unitsSwitch_state_set),
        NULL
    );
    on_unitsSwitch_state_set(unitsSwitch);

    // Get color mode switch and set handler.
    GtkSwitch *colorSwitch = GTK_SWITCH(gtk_builder_get_object(builder, "colorSwitch"));
    g_signal_connect
    (
        colorSwitch,
        "state-set",
        G_CALLBACK(on_colorSwitch_state_set),
        NULL
    );

    // Get dropdown menu and connect handler for new update interval selection.
    GtkDropDown *intervalDropdown = GTK_DROP_DOWN(gtk_builder_get_object(builder, "intervalDropdown"));
    g_signal_connect
    (
        intervalDropdown,
        "notify::selected",
        G_CALLBACK(on_intervalDropdown_notify_selected),
        NULL
    );    

    // Free builder - it is no longer needed.
    g_object_unref(builder);

    /* Set starting values and placeholders to display and gather initial data.
       If failure is returned, show window with error message. */
    if (!startup_prepare_display())
    {
        show_error_message(&primaryBox);
        gtk_widget_show(window);

        return;
    }

    /* Set timeout at default interval (default menu selection).
       timeout_update_display() will be triggered each interval until it
       returns FALSE, user exits program, or timeout is manually destroyed. */
    on_intervalDropdown_notify_selected(intervalDropdown);

    // Unhide user interface.
    gtk_widget_show(window);

    // Update update variable to indicate completion of startup tasks.
    Ready = true;
}

bool startup_prepare_display()
{
    // Gather first round of data for cpu use calculation.
    CPUs cpuData[MAX_PROC] = {0};
    double cpuUse_aggregate = 0;
    int prc_count = 0;
    if (!populate_cpu_use(cpuData, &cpuUse_aggregate, &prc_count))
    {
        show_error_message(&primaryBox);
        return FALSE;
    }

    // Set text containing to expander titles, with placeholders where values will be displayed.
    char *cpu_title_str = g_strdup_printf(titleTemplate, "CPU", "...", "%");
    gtk_expander_set_label(cpuBox.expander, cpu_title_str);
    g_free(cpu_title_str);
    char *mem_title_str = g_strdup_printf(titleTemplate, "MEM", "...", "GB");
    gtk_expander_set_label(memBox.expander, mem_title_str);
    g_free(mem_title_str);

    /* Set placeholder text for values within collapsed CPU and memory detail expanders
       to avoid displaying blank areas show when either expander is opened. */
    on_expander_notify_expanded(cpuBox.expander);
    on_expander_notify_expanded(memBox.expander);

    /* Open virtual file containing value of total installed system memory in
       kilobytes, returning failure if the file could not be found or opened. */
    FILE *file = fopen("/proc/meminfo", "r");
    if (file == NULL)
    {
        return false;
    }

    /* Read first file line to a buffer, then close the file. */
    char buffer[MAX_LINE_MEMINFO];
    fgets(buffer, sizeof buffer, file);
    fclose(file);

    /* find index of first numeric character in buffer,
       returning failure if the buffer contains no numeric values. */
    size_t cursor = 0;
    if (!advance_cursor_to_digit(buffer, &cursor))
    {
        fclose(file);
        return false;
    }

    // Pick characters comprising the value from buffer.
    char tmp[MEM_CHAR] = {0};
    int charCount = 0;
    while (isdigit(buffer[cursor]) && charCount < MEM_CHAR - 1)
    {
        tmp[charCount] = buffer[cursor + charCount];
        charCount++;
    }

    /* Convert the characters obtained to a numeric value and store in global variable.
       This value will remain unchanged through duration of app runtime. */
    mem_totalInstalled = strtod(tmp, NULL);

    // Formulate text for total hardware memory in gigabytes and set as footer inside memory expander.
    char *mem_total_str = g_strdup_printf("installed: %.1f GB", 0.000001 * mem_totalInstalled);
    gtk_label_set_label(memBox.totalInstalled, mem_total_str);
    g_free(mem_total_str);

    // Return success.
    return true;
}

// Function updates information displayed in application window.
gboolean timeout_update_display()
{
    /* Check whether or not each expander is expanded, setting global variables accordingly.
       These will not change for the remainder of the interval. */
    Expanded_cpu = gtk_expander_get_expanded(cpuBox.expander);
    Expanded_mem = gtk_expander_get_expanded(memBox.expander);

    /* Gather cpu use and memory data.
       If failure is returned, show error message and return FALSE to destroy timeout. */
    CPUs cpuData[MAX_PROC] = {0};
    double cpuUse_aggregate = 0;
    int prc_count = 0;
    double memVals[MEM_VALS] = {0};
    if ((!populate_cpu_use(cpuData, &cpuUse_aggregate, &prc_count)) ||
       (!populate_mem_vals(memVals)))
    {
        show_error_message(&primaryBox);
        return FALSE;
    }

    // Update values in expander titles.
    set_expander_title(cpuBox.expander, "CPU", titleTemplate, cpuUse_aggregate, 1, "%");
    set_expander_title(memBox.expander, "MEM", titleTemplate, memVals[0], memVal_factor, unit);

    // Update values within open expander(s).
    if (Expanded_cpu)
    {
        /* Gather cpu frequency data. If failure is returned or the quantity of frequencies
           varies from what is expected, show error message and return FALSE to destroy timeout. */
        int frq_count = 0;
        if ((!populate_cpu_frqs_ids(cpuData, &frq_count)) ||
            (frq_count != prc_count))
        {
            show_error_message(&primaryBox);
            return FALSE;
        }

        // Clear grid of all current values.
        grid_clear(cpuBox.grid);

        // Check for hyperthreading and repopulate grid based on results.
        if (hyper_thread(cpuData, prc_count))
        {
            // Sort data in order of increasing core ID so threads are grouped by core.
            sort_cpu_data(cpuData, prc_count);

            // Repopulate grid, displaying core ID only once pe thread pair.
            for(int proc = 0; proc < prc_count; proc++)
            {
                if (proc % 2 == 0)
                {
                    new_grid_label_int(cpuBox.grid, cpuData[proc].coreID, 0, proc, 1, 2);
                }
                else
                {
                    gtk_grid_attach(cpuBox.grid, gtk_label_new(NULL), 0, proc, 1, 2);
                }
                new_grid_label_int(cpuBox.grid, cpuData[proc].processor, 1, proc, 1, 1);
                new_grid_label_float(cpuBox.grid, cpuData[proc].use, 2, proc, 2, 1);
                gtk_grid_attach(cpuBox.grid, gtk_label_new(cpuData[proc].frequency), 3, proc, 3, 1);
            }
        }
        else
        {
            // Repopulate grid with uniform rows.
            for(int proc = 0; proc < prc_count; proc++)
            {
                new_grid_label_int(cpuBox.grid, cpuData[proc].coreID, 0, proc, 1, 1);
                new_grid_label_int(cpuBox.grid, cpuData[proc].processor, 1, proc, 1, 1);
                new_grid_label_float(cpuBox.grid, cpuData[proc].use, 2, proc, 2, 1);
                gtk_grid_attach(cpuBox.grid, gtk_label_new(cpuData[proc].frequency), 3, proc, 3, 1);
            }
        }
    }
    if (Expanded_mem)
    {
        set_mem_label(memVals[1], memBox.buffers, memVal_format, memVal_factor);
        set_mem_label(memVals[2], memBox.pageCache, memVal_format, memVal_factor);
        set_mem_label(memVals[3], memBox.swapCache, memVal_format, memVal_factor);
    }

    // Return success.
    return TRUE;
}

// Function determines memory usage and populates array with results.
bool populate_mem_vals(double memVals[])
{
    // Determine how many values are needed this interval.
    int maxVals;
    if (Expanded_mem)
    {
        maxVals = MEM_VALS;
    }
    else
    {
        maxVals = 1;
    }

    // Open virtual file containing current memory details in kilobytes.
    FILE *file = fopen("/proc/meminfo", "r");
    if (file == NULL)
    {
        return false;
    }

    /* Create buffer. Read in and discard first two file lines. Continue reading file
       line(s) to gather data, stopping when all values needed have been obtained. */
    char buffer[MAX_LINE_MEMINFO];
    fgets(buffer, sizeof buffer, file);
    fgets(buffer, sizeof buffer, file);
    for (int valNo = 0; valNo < maxVals; valNo++)
    {
        /* Read file line to buffer and advance cursor to first numeric character,
           returning failure if the buffer contains no numeric characters. */
        fgets(buffer, sizeof buffer, file);
        size_t cursor = 0;
        if (!advance_cursor_to_digit(buffer, &cursor))
        {
            fclose(file);
            return false;
        }

        // Pick characters comprising the value from the buffer.
        char tmp[MEM_CHAR] = {0};
        int charCount = 0;
        while (charCount < MEM_CHAR - 1 && isdigit(buffer[cursor]))
        {
            tmp[charCount] = buffer[cursor + charCount];
            charCount++;
        }

        /* Convert characters to numeric value and store in the given array.
           Values stored will be, in order:  available, buffers, page, and swap. */
        memVals[valNo] = strtod(tmp, NULL);
    }

    // Close the file.
    fclose(file);

    /* Convert available memory to total use (use = total - available)
       Return failure if result indicates error(s) in values obtained from file. */
    memVals[0] = mem_totalInstalled - memVals[0];
    if (memVals[0] <= 0)
    {
        return false;
    }

    // Return success.
    return true;
}

// Function determines cpu usage over update interval and populates the given array with results.
bool populate_cpu_use(CPUs cpuData[], double *cpuUse_aggregate, int *prc_count)
{
    /* Open virtual file. File contains series of time values in
       seconds & reflects partitioning of cpu activity since boot. */
    FILE *file = fopen("/proc/stat", "r");
    if (file == NULL)
    {
        return false;
    }

    /* Persistent arrays for usage calculation based on old and current values.
       Each will contain an aggregate value (index 0) as well as one value per processor. */
    static double idles[MAX_PROC + 1];
    static double totals[MAX_PROC + 1];

    /* Read file lines, for each line isolating the data needed and doing calculations.
       Stop when all relevant lines (those beginning with "cpu") have been read. */
    int lineNo = 0;
    char buffer[MAX_LINE_STAT];
    while (fgets(buffer, sizeof buffer, file) != NULL && strstr(buffer, "cpu") != NULL)
    {
        /* Discard trailing newline from buffer, returning failure if trailing
           newline is not found (i.e., if line size exceeds buffer size) */
        int end = strlen(buffer) - 1;
        if (buffer[end] != '\n')
        {
            fclose(file);
            return false;
        }
        buffer[end] == '\0';

        // Set aside total & idle time since boot from last time this function was called
        double idle_old = 0;
        double total_old = 0;
        if (Ready)
        {
            idle_old = idles[lineNo];
            total_old = totals[lineNo];
        }

        /* Discard first word (i.e. "cpu", "cpu0", "cpu1", ...) from buffer,
           then parse out remaining words (numeric values), updating total (sum)
           and idle time since boot in totals and idles arrays, respectively. */
        strtok(buffer, " ");
        char *str = strtok(NULL, " ");
        totals[lineNo] = strtod(str, NULL);
        int valCount = 1;
        while ((str = strtok(NULL, " ")) != NULL)
        {
            double val = strtod(str, NULL);
            if (valCount == 3)  // 4th numeric column of /proc/stat
            {
                idles[lineNo] = val;
            }
            totals[lineNo] += val;
            valCount++;
        }

        /* Calculate cpu usage percent over interval according to:
           100% * (1 - (change in idle time) / (change in total time)).
           If cpu expander was not expanded at start of interval, then keep the data gathered
           to the idles and totals arrays but skip usage calculation for collapsed content. */
        if (Ready)
        {
            if (lineNo == 0)
            {
                // calculation from aggregate values
                *cpuUse_aggregate = 100 * (1 - ((idles[lineNo] - idle_old) / (totals[lineNo] - total_old)));
            }
            else if (Expanded_cpu)
            {
                // calculation for unique processor
                cpuData[lineNo - 1].use = 100 * (1 - ((idles[lineNo] - idle_old) / (totals[lineNo] - total_old)));
            }
        }
        lineNo++;
    }

    // Close the file.
    fclose(file);

    /* Initialize variable at given address with number of processors
       indicated by number of lines read from file, not including first line. */
    *prc_count = lineNo - 1;

    // Return success.
    return true;
}

/* Function populates given array with frequencies and informs of the total
   number of frequencies. Returns true for success or false for failure. */
bool populate_cpu_frqs_ids(CPUs cpuData[], int *frq_count)
{
    /* Open virtual file; file contains cpu details including core IDs,
       associated logical processors, and their current frequencies. */
    FILE *file = fopen("/proc/cpuinfo", "r");
    if (file == NULL)
    {
        return false;
    }

    /* Create buffer of size > maximum expected for lines of interest. */
    char buffer[MAX_LINE_CPUINFO];

    /* Create counter variables to determine total number
       of logical processors and check for inconsistencies. */
    int procCount = 0;
    int frqCount = 0;
    int idCount = 0;

    while (fgets(buffer, sizeof buffer, file) != NULL)
    {
        /* Determine if lines contains a value of interest. If not, read next
           line to buffer such that long flags lines are handled by successive
           iterations (fgets reads reads to newline and/or (buffer size - 1) */
        bool Frequency = false;
        bool Processor = false;
        if (strstr(buffer, "cpu MHz") != NULL)
        {
            Frequency = true;
        }
        else if (strstr(buffer, "processor") != NULL)
        {
            Processor = true;
        }
        else if (strstr(buffer, "core id") == NULL)
        {
            continue;
        }
        
        // Ensure line contains numeric character(s) and advance cursor to first instance.
        size_t cursor = 0;
        if (!advance_cursor_to_digit(buffer, &cursor))
        {
            fclose(file);
            return false;
        }
        /* Pick characters comprising value from line and store to struct array. */
        int charNo = 0;
        if (Frequency)
        {
            // Store frequencies as-is without conversion to numeric value.
            while (isdigit(buffer[cursor]) && charNo < FRQ_CHAR - 1)
            {
                cpuData[frqCount].frequency[charNo] = buffer[cursor];
                charNo++;
                cursor++;
            }
            frqCount++;
        }
        else
        {
            /* Store processor numbers and core IDs as integer values,
               as this will be needed to check for hyperthreading and sort the cpu data array.
               Return failure if the value exceeds 99. */
            char tmp[4] = {0};
            while (charNo < 3 && isdigit(buffer[cursor]))
            {
                tmp[charNo] = buffer[cursor];
                charNo++;
                cursor++;
            }
            if (charNo == 2)
            {
                fclose(file);
                return false;
            }
            if (Processor)
            {
                cpuData[procCount].processor = atoi(tmp);
                procCount++;
            }
            else
            {
                cpuData[idCount].coreID = atoi(tmp);
                idCount++;
            }
        }
    }

    // Close the file.
    fclose(file);

    /* Ensure all expected data was gathered from file by confirming that processor
       numbers, core IDs, and frequencies were all gathered in the same quantity,
       and update total number of frequencies at given address. */
    if (procCount != idCount || idCount != frqCount)
    {
        return false;
    }
    *frq_count = frqCount;

    // Return success.
    return true;
}

// Function resets transient content within the the given expander to placeholder(s).
static void on_expander_notify_expanded(GtkExpander *expander)
{
    /* If function was triggered by collapsing of the expander, replace
       the values within by placeholder(s) to avoid displaying outdated
       information or blank areas when the expander is re-opened. */
    if (!gtk_expander_get_expanded(expander))
    {
        /* If the expander is the CPU expander, clear grid contents and insert
           new row containing placeholder text. Otherwise, it is the memory
           expander; replace current text of value labels with placeholders. */
        if (expander == cpuBox.expander)
        {
            grid_clear(cpuBox.grid);
            gtk_grid_attach(cpuBox.grid, gtk_label_new("Please wait..."), 0, 0, 1, 1);
        }
        else
        {
            gtk_label_set_label(memBox.buffers, "...");
            gtk_label_set_label(memBox.pageCache, "...");
            gtk_label_set_label(memBox.swapCache, "...");
        }
    }
}

// Function changes display update rate.
static void on_intervalDropdown_notify_selected(GtkDropDown *intervalDropdown)
{
    // Destroy current timeout loop.
    if (timeout_ID != 0)
    {
        g_source_remove(timeout_ID);
    }

    // Get string from current menu selection.
    GObject *selected_obj = gtk_drop_down_get_selected_item(intervalDropdown);
    char *obj_str;
    g_object_get(selected_obj, "string", &obj_str, NULL);

    // Do units conversion from s to ms and set new timeout.
    timeout_ID = g_timeout_add((int)(atof(obj_str) * 1000), timeout_update_display, NULL);

    // Free the string.
    g_free(obj_str);
}

// Function switches memory display units between gigabytes and percentages.
static void on_unitsSwitch_state_set(GtkSwitch *unitsSwitch)
{
    /* Set conversion factor, format specifier, and units to be used
       to display memory information based on current switch position -
       "ON" indicates percentages, while "OFF" indicates gigabytes. */
    if (gtk_switch_get_active(unitsSwitch))
    {
        memVal_factor = 100 / mem_totalInstalled;
        strcpy(memVal_format, "%.1f %%");
        strcpy(unit, "%");
    }
    else
    {
        memVal_factor = 0.000001;
        strcpy(memVal_format, "%.2f GB");
        strcpy(unit, "GB");
    }
}

// Function switches color scheme between light and dark modes.
static void on_colorSwitch_state_set(GtkSwitch *colorSwitch)
{
    /* Check position of dark mode switch, then load alternative style sheet based on result.
       Note that this clears any previously loaded style sheet from the provider. */
    if (gtk_switch_get_active(colorSwitch))
    {
        gtk_css_provider_load_from_file
        (
            cssProvider_color,
            g_file_new_for_path("/usr/share/easymon/css/easymon-dark-scheme.css")
        );
    }
    else
    {
        gtk_css_provider_load_from_file
        (
            cssProvider_color,
            g_file_new_for_path("/usr/share/easymon/css/easymon-light-scheme.css")
        );
    }
}

