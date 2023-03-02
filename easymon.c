/*_____________________________________________________________________________
 |                                                                             |
 |                                  "EasyMon"                                  |
 |_____________________________________________________________________________|
   System monitor for Linux - Written in C using GTK-4.

      Author: C.T. Bishop
      Origin: https://github.com/ctbishop/EasyMon


      PLEASE NOTE:
      This app is a prototype, and is not guaranteed to be compatible with all
      systems or Linux distributions, or to otherwise function as intended.
      That being said, you are welcome to compile this source code and use
      the EasyMon program however it may suit you. I only ask that you credit
      the work appropriately where and when this may apply, including for any
      modified forms. Please report bugs, issues, and/or feedback via the
      EasyMon GitHub repository, linked above.

 ______________________________________________________________________________*/


#include <ctype.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Display update rate, in seconds.
#define UPDATE_INTERVAL 1

// Maximum logical processors accomodated by program.
#define MAX_PROC 16

/* Number of memory values to be displayed,
   not including total hardware capacity. */
#define MEM_VALS 4

// String sizing macros.
#define FRQ_CHAR 5
#define MEM_CHAR 10
#define CPU_SEG_CHAR 40

// Structure to be used in CPU data array.
struct CPUs
{
    float use;
    int processor;
    char frequency[FRQ_CHAR];
    int coreID;
};

// Total hardware memory capacity, to be determined only once.
float memTotalHardware = 0;
char mem_total_hardware[30] = {0};

// Defaults; "%" checkbox is not activated.
char mem_unit[] = " GB";
bool Percentages = false;

// Default; display is collapsed.
bool Expanded = false;

// Default; turboboost/hyperthreading not active.
bool TurboBoost = false;

/* Initial; true after first round of cpu and memory data has
   been gathered and initial determinations have been made. */
bool Ready = false;

// Function Prototypes.
void switch_mem_units(GtkCheckButton *chkbx);
void switch_detail(GtkSwitch *swtch, gpointer user_data);
void formulate_cpu_detail_text(char cpu_detail[], struct CPUs cpuData[], int processors);
void sort_cpu_data(struct CPUs cpuData[], int processors);
void turbo_boost(struct CPUs cpuData[], int processors);
void formulate_mem_text(char mem_strs[][MEM_CHAR], float memVals[]);
void show_error_message(GtkTextBuffer *buff);
bool advance_cursor_to_digit(char *buffer, size_t *cursor);
bool populate_cpu_frqs_ids(struct CPUs cpuData[], int *frequencies);
bool populate_cpu_use(struct CPUs cpuData[], double *cpuUseOverall, int *processors);
bool populate_mem_vals(float memVals[]);
gboolean refresh_display_text(gpointer user_data);
void setup_make_gui(GtkApplication *app, gpointer user_data);

// CREATE, LAUNCH, & STOP GTK APPLICATION. 
int main(int argc, char **argv)
{  
    // Create application with identifying string.
    GtkApplication *app = gtk_application_new("com.gmail.ctbishop34.EasyMon", G_APPLICATION_FLAGS_NONE);

    // Connect activation to make_build_gui function.
    g_signal_connect(app, "activate", G_CALLBACK(setup_make_gui), NULL);

    /* Launch application; emits "activate" signal, triggering make_build_gui.
       Wait for return value indicating that user has closed the application. */
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    // Free closed application from memory.
    g_object_unref(app);

    return status;
}

// ASSEMBLE GUI AND SET FUNCTION TRIGGERS.
void setup_make_gui(GtkApplication *app, gpointer user_data)
{
    // Create window with title and initial size.
    GtkWidget *window = gtk_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_title(GTK_WINDOW(window), "EasyMon");
    gtk_window_set_default_size(GTK_WINDOW(window), 280, 230);

    // Connect window to EasyMon application.
    gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(app));

    // Create decorative window border and set to window.
    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_window_set_child(GTK_WINDOW(window), frame);

    /* Create container to which items will be appended vertically.
       Place in frame such that container occupies all available space. */
    GtkWidget *box1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_frame_set_child(GTK_FRAME(frame), box1);

    /* Create container to which items will be horizontally appended.
       Set margins and place in vertical container. */
    GtkWidget *box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_margin_top(box2, 2);
    gtk_widget_set_margin_bottom(box2, 2);
    gtk_widget_set_margin_start(box2, 185);
    gtk_widget_set_margin_end(box2, 2);
    gtk_box_append(GTK_BOX(box1), box2);

    /* Create formatted label for "Expand" switch.
       Place in horizontal container. */
    GtkWidget *label1 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label1), "<small><span foreground=\"gray\">Expand  </span></small>");
    gtk_box_append(GTK_BOX(box2), label1);

    /* Create "Expand" switch.
       Place as second item in horizontal container, to left of switch. */
    GtkWidget *swtch = gtk_switch_new();
    gtk_box_append(GTK_BOX(box2), swtch);

    /* Link "Expand" switch to switch_detail function;
       trigger function when user toggles switch. */
    g_signal_connect(swtch, "state-set", G_CALLBACK(switch_detail), NULL);

    /* Create text area and hide blinking cursor.
       Place as second item in vertical container, below horizontal container. */
    GtkWidget *txt = gtk_text_view_new();
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(txt), FALSE);
    gtk_box_append(GTK_BOX(box1), txt);

    // Get buffer for text area and assign custom formatting tags.
    GtkTextBuffer *buff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txt));
    gtk_text_buffer_create_tag(buff, "all", "editable", FALSE, "family", "liberation mono", NULL);
    gtk_text_buffer_create_tag(buff, "pad-above", "pixels_above_lines", 15, NULL);
    gtk_text_buffer_create_tag(buff, "header", "foreground", "#e00000", "pixels_below_lines", 5, NULL);
    gtk_text_buffer_create_tag(buff, "leader", "scale", PANGO_SCALE_X_LARGE, "foreground", "#30cdff", NULL);
    gtk_text_buffer_create_tag(buff, "unit", "scale", PANGO_SCALE_SMALL, "foreground", "#30cdff", NULL);
    gtk_text_buffer_create_tag(buff, "title", "scale", PANGO_SCALE_LARGE, "weight", PANGO_WEIGHT_BOLD,
                                              "underline", PANGO_UNDERLINE_SINGLE, "foreground", "#2aa200", NULL);

    /* Create formatted label for "%" checkbox.
       Place at coordinates using overlay. */
    GtkWidget *label2 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label2), "<span font_weight=\"ultralight\" foreground=\"gray\">%</span>");
    gtk_text_view_add_overlay(GTK_TEXT_VIEW(txt), label2, 48, 370);

    /* Create "%" checkbox.
       Place at coordinates next to label using overlay. */
    GtkWidget *chkbx = gtk_check_button_new();
    gtk_text_view_add_overlay(GTK_TEXT_VIEW(txt), chkbx, 60, 366);

    /* Link "%" checkbox to switch_mem_units function;
       trigger function when user clicks checkbox. */
    g_signal_connect(chkbx, "toggled", G_CALLBACK(switch_mem_units), NULL);

    // Set initial text to text area.
    refresh_display_text(buff);

    /* Set timeout for refresh_display_text function;
       trigger function at interval defined by macro. */
    g_timeout_add_seconds(UPDATE_INTERVAL, refresh_display_text, buff);

    // Make GUI visible to user.
    gtk_widget_show(window);
}

// TRIGGERED ONCE PER UPDATE INTERVAL DEFINED BY MACRO; SET/REFRESH DISPLAY TEXT.
gboolean refresh_display_text(gpointer user_data)
{
    // Get text buffer with formatting tags from user data passed to function.
    GtkTextBuffer *buff = GTK_TEXT_BUFFER(user_data);

    // Populate data arrays & check success.
    float memVals[MEM_VALS];
    struct CPUs cpuData[MAX_PROC] = {0};
    double cpuUseOverall;
    int processors;
    int frequencies;
    if ((!populate_mem_vals(memVals)) ||
       (!populate_cpu_use(cpuData, &cpuUseOverall, &processors)) ||
       (!populate_cpu_frqs_ids(cpuData, &frequencies)) ||
       (frequencies != processors))
    {
        show_error_message(buff);
        return FALSE;  // (function will not be re-triggered)
    }

    // Formulate array of strings for memory display.
    char mem_strs[MEM_VALS][MEM_CHAR] = {0};
    formulate_mem_text(mem_strs, memVals);

    /* If needed, formulate text for total hardware memory, assign placeholder to
       CPU text, and determine whether or not turboboost is active. If these were
       done previously, sort CPU data if needed and formulate text for CPU display. */
    char cpu_leader[7] = {0};  // i.e., " xxx.x"
    char cpu_detail[MAX_PROC * CPU_SEG_CHAR] = {0};  //
    if(Ready)
    {
        sprintf(cpu_leader, " %.1f", cpuUseOverall);
        if(Expanded)
        {
            if (TurboBoost)
                sort_cpu_data(cpuData, processors);
            formulate_cpu_detail_text(cpu_detail, cpuData, processors);
        }
    }
    else
    {
        sprintf(mem_total_hardware, "\n\n                 /%.1f GB", memTotalHardware / 1000000);
        strcpy(cpu_leader, " ...");  // placeholder
        turbo_boost(cpuData, processors);
        Ready = true;
    }

    // Clear existing text from display.
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buff, &start, &end);
    gtk_text_buffer_delete(buff, &start, &end);

    // Display cpu title label and overall cpu use percentage.
    gtk_text_buffer_insert_with_tags_by_name(buff, &end, "\n\n   ", -1, "all", NULL);
    gtk_text_buffer_insert_with_tags_by_name(buff, &end, "CPU          ", -1, "title", "all", NULL);
    gtk_text_buffer_insert_with_tags_by_name(buff, &end, cpu_leader, -1, "leader", "all", NULL);
    gtk_text_buffer_insert_with_tags_by_name(buff, &end, " %\n", -1, "unit", "all", NULL);

    // Display cpu detail: use percentages and frequencies by core and/or logical processor.
    if(Expanded)
    {
        gtk_text_buffer_insert_with_tags_by_name(buff, &end, "      Cr Pr   Use%   MHz\n", -1, "header", "pad-above", "all", NULL);
        gtk_text_buffer_insert_with_tags_by_name(buff, &end, cpu_detail, -1, "all", NULL);
    }
    // Display memory title label total memory use.
    gtk_text_buffer_insert_with_tags_by_name(buff, &end, "\n\n   ", -1, "all", NULL);
    gtk_text_buffer_insert_with_tags_by_name(buff, &end, "RAM          ", -1, "title","all", NULL);
    gtk_text_buffer_insert_with_tags_by_name(buff, &end, mem_strs[0], -1, "leader", "all", NULL);
    gtk_text_buffer_insert_with_tags_by_name(buff, &end, mem_unit, -1, "unit", "all", NULL);
    gtk_text_buffer_insert_with_tags_by_name(buff, &end, "\n", -1, "all", NULL);

    /* Display memory detail: usage for buffers, page cache, and swap cache.
       Show total memory capacity below detail. */
    if(Expanded)
    {
        const char mem_headers[][25] = {{"\n             buff "}, {"\n             page "}, {"\n             swap "}};
        for (int i = 1; i < MEM_VALS; i++)
        {
            gtk_text_buffer_insert_with_tags_by_name(buff, &end, mem_headers[i-1], -1, "header", "all", NULL);
            gtk_text_buffer_insert_with_tags_by_name(buff, &end, mem_strs[i], -1, "all", NULL);
            gtk_text_buffer_insert_with_tags_by_name(buff, &end, mem_unit, -1, "all", NULL);
        }
        gtk_text_buffer_insert_with_tags_by_name(buff, &end, mem_total_hardware, -1, "header", "all", NULL);
    }

    return TRUE;
}

// DETERMINE MEMORY USAGE & POPULATE ARRAY WITH RESULTS.
bool populate_mem_vals(float memVals[])
{
    /* Open pseudofile. File contains current memory details
       in units of kilobytes */
    FILE *file = fopen("/proc/meminfo", "r");
    if (file == NULL)
        return false;

    /* Read file lines to buffer of size > maximum expected line
       size, until all values needed have been gathered. */
    for (int lineNo = 0, valNo = 0; valNo < MEM_VALS; lineNo++)
    {
        char buffer[50];
        fgets(buffer, sizeof buffer, file);

        /* Skip second line (free memory), as well as first
           line (total hardware memory) if already determined. */
        if ((lineNo == 1) ||
           (lineNo == 0 && Ready))
            continue;

        // Advance cursor to first numeric character.
        size_t cursor = 0;
        if (!advance_cursor_to_digit(buffer, &cursor))
        {
            fclose(file);
            return false;  // (line contains no numeric characters)
        }
        /* Pick characters comprising value from buffer, not to
           exceed maximum number of characters defined by macro */
        char tmp[MEM_CHAR] = {0};
        int charCount = 0;
        while (charCount < MEM_CHAR - 1 && isdigit(buffer[cursor]))
        {
            tmp[charCount] = buffer[cursor + charCount];
            charCount++;
        }
        /* Store values for available memory, buffers, page cache,
           and swap cache in that order in array. If not yet done, also
           store value of total hardware memory to separate variable. */
        if (lineNo != 0)
        {
            memVals[valNo] = strtod(tmp, NULL);
            valNo++;
        }
        else
            memTotalHardware = strtod(tmp, NULL); 
    }
    fclose(file);

    /* Convert available memory to total memory usage:
       usage = total - available memory */
    memVals[0] = memTotalHardware - memVals[0];

    if (memVals[0] <= 0)
        return false;  // (erroneous values were obtained)

    return true;
}

/* DETERMINE CPU USAGE OVER UPDATE INTERVAL, OVERALL & FOR
   INDIVIDUAL PROCESSORS, AND POPULATE STRUCT ARRAY WITH RESULTS.*/
bool populate_cpu_use(struct CPUs cpuData[], double *cpuUseOverall, int *processors)
{
    /* Open pseudofile. File contains series of time values in
       seconds & reflects partitioning of cpu activity since boot. */
    FILE *file = fopen("/proc/stat", "r");
    if (file == NULL)
        return false;

    /* Create static variables for usage calculation, each to contain
       an aggregate value (index 0) as well as one value per processor. */
    static double idles[MAX_PROC + 1];
    static double totals[MAX_PROC + 1];

    // Create buffer of > maximum expected line size.
    char buffer[100];

    // Read file lines to buffer until all relevant lines are read.
    int count = 0;
    while ((fgets(buffer, sizeof buffer, file) != NULL) &&
           (strstr(buffer, "cpu") != NULL))
    {
        if (!Expanded && count > 0)
        {
            count++;
            continue;
        }
        /* Set aside total & idle time since boot for processor from
           last time function was triggered (see update interval macro). */
        double idle;
        double total;
        if (Ready)
        {
            idle = idles[count];
            total = totals[count];
        }
        // Discard trailing newline from buffer.
        int end = strlen(buffer) - 1;
        if (buffer[end] != '\n')  // (line size exceeds maximum)
        {
            fclose(file);
            return false;
        }
        buffer[end] == '\0';

        // Discard line label as first word from buffer.
        strtok(buffer, " ");

        /* Parse time interval values from buffer. Recalculate total time
           since boot as the sum of all values & pick out current idle time. */
        char *str = strtok(NULL, " ");
        totals[count] = strtod(str, NULL);
        int valCount = 1;
        while ((str = strtok(NULL, " ")) != NULL)
        {
            if (valCount == 3)  // 4th numeric column of /proc/stat
                idles[count] = strtod(str, NULL);
            totals[count] += strtod(str, NULL);
            valCount++;
        }
        /* Calculate cpu usage over update interval according to:
           100% * (1 - (change in idle time) / (change in total time)). */
        if (Ready)
        {
            if (count == 0)  // calculation from aggregate values
                *cpuUseOverall = 100 * (1 - ((idles[count] - idle) / (totals[count] - total)));
            else  // calculation for processor
                cpuData[count - 1].use = 100 * (1 - ((idles[count] - idle) / (totals[count] - total)));
        }
        count++;
    }
    fclose(file);

    *processors = count - 1;
    return true;
}

// POPULATE STRUCT ARRAY WITH PROCESSOR/CORE IDS AND FREQUENCIES.
bool populate_cpu_frqs_ids(struct CPUs cpuData[], int *frequencies)
{
    /* Open pseudofile; file contains cpu details including core IDs,
       associated logical processors, and their current frequencies. */
    FILE *file = fopen("/proc/cpuinfo", "r");
    if (file == NULL)
        return false;

    /* Create buffer of size > maximum expected for lines of interest. */
    char buffer[30];

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
            Frequency = true;
        else if (strstr(buffer, "processor") != NULL)
            Processor = true;
        else if (strstr(buffer, "core id") == NULL)
            continue;
        
        // Advance cursor to value.
        size_t cursor = 0;
        if (!advance_cursor_to_digit(buffer, &cursor))
        {
            fclose(file);
            return false;  // (line contains no numeric values)
        }
        /* Pick characters comprising value from line and store to struct array
           either as-is (frequencies) or as numeric value (processor, core id). */
        int charNo = 0;
        if (Frequency)
        {
            while (isdigit(buffer[cursor]) && charNo < FRQ_CHAR - 1)
            {
                cpuData[frqCount].frequency[charNo] = buffer[cursor];
                charNo++;
                cursor++;
            }
            frqCount++;
        }
        else  // processor or core id
        {
            char tmp[3] = {0};  // max digits = 2
            while (charNo < 2 && isdigit(buffer[cursor]))
            {
                tmp[charNo] = buffer[cursor];
                charNo++;
                cursor++;
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
    fclose(file);

    if (procCount != idCount || idCount != frqCount)
        return false;  // (missing/excess values)

    *frequencies = frqCount;
    return true;
}

// FIND INDEX OF FIRST NUMERIC VALUE IN GIVEN BUFFER.
bool advance_cursor_to_digit(char *buffer, size_t *cursor)
{
    int a = *cursor;
    while (!isdigit(buffer[a]))
    {
        if (buffer[a] == '\0')
            return false;  // (buffer contains no numeric values)
        a++;
    }
    *cursor = a;
    return true;
}

// PRINT ERROR MESSAGE TO DISPLAY.
void show_error_message(GtkTextBuffer *buff)
{
    // Clear any/all text from display.
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buff, &start, &end);
    gtk_text_buffer_delete(buff, &start, &end);

    // Show error message.
    gtk_text_buffer_insert(buff, &end, "\n\n Sorry..."
                                       "\n an unexpected error occurred."
                                       "\n\n Please report issues at:"
                                       "\n https://github.com/ctbishop/easymon\n\n", -1);
}

// POPULATE ARRAY OF STRINGS FOR MEMORY DISPLAY.
void formulate_mem_text(char mem_strs[][MEM_CHAR], float memVals[])
{
    /* Do units conversion from kB to either gigabytes or percentage of total
       total capacity as formatted strings are written to array. If display
       is collapsed, do only for total memory usage. Otherwise, also include
       buffers, page cache, & swap cache. */
    if (Percentages)
    {
        sprintf(mem_strs[0], " %d", (int)(100 * (memVals[0] / memTotalHardware)));
        if (Expanded)
            for (int valNo = 1; valNo < MEM_VALS; valNo++)
                    sprintf(mem_strs[valNo], " %d", (int)(100 * (memVals[valNo] / memTotalHardware)));
    }
    else
    {
        sprintf(mem_strs[0], " %.1f", memVals[0] / 1000000);
        if (Expanded)
            for (int valNo = 1; valNo < MEM_VALS; valNo++)
                sprintf(mem_strs[valNo], " %.1f", memVals[valNo] / 1000000);
    }
}

// DETERMINE WHETHER OR NOT TURBOBOOST IS ON.
void turbo_boost(struct CPUs cpuData[], int processors)
{
    /* Check for inequality between coreID and processor number.
       If found, assume active turboboost/hyperthreading. */
    for (int prc = 0; prc < processors; prc++)
    {
        if (cpuData[prc].coreID != cpuData[prc].processor)
        {
            TurboBoost = true;
            return;
        }
    }
    TurboBoost = false;
}

// SORT CPU DATA IN ORDER OF INCREASING CORE ID.
void sort_cpu_data(struct CPUs cpuData[], int processors)
{
    /* Iterate Through All Pairs of Adjacent Elements, Swapping Their Positions
       if Needed. Repeat Process Until a Full Pass is Made Without any Swaps */
    bool swap = true;
    while (swap)
    {
        swap = false;
        for (int first = 0, second = 1; second < processors; first++, second++)
            if (cpuData[first].coreID > cpuData[second].coreID)
            {
                struct CPUs tmp = cpuData[first];
                cpuData[first] = cpuData[second];
                cpuData[second] = tmp;
                swap = true;
            }
    }
}

// BUILD STRING CONTAINING ALL TEXT FOR CPU DETAIL.
void formulate_cpu_detail_text(char cpu_detail[], struct CPUs cpuData[], int processors)
{
    /* Iteratively build master string by creating formatted segments, one for
       each logical processor, and appending segments to master string as they
       are produced. Account for varying number of characters occupied by CPU
       use percentage by including more or less whitespace*/

    /* If turboboost (2 threads/core) is enabled, all CPU data was previously
       sorted by core ID; add extra newline between each pair originating
       from the same core, and include core ID no. only once per pair. */
    if (TurboBoost)
        for(int proc = 0; proc < processors; proc++)
        {
            char tmp[CPU_SEG_CHAR];
            if (proc % 2 == 0)  // first thread within a pair
            {
                if (cpuData[proc].use < 10)
                    sprintf(tmp, "      %d  %d    %.1f    %s\n", cpuData[proc].coreID, cpuData[proc].processor,
                                                                 cpuData[proc].use, cpuData[proc].frequency);
                else
                    sprintf(tmp, "      %d  %d    %.1f   %s\n", cpuData[proc].coreID, cpuData[proc].processor,
                                                                cpuData[proc].use, cpuData[proc].frequency);
            }
            else if (proc != processors - 1)  // second thread within a pair
            {
                if (cpuData[proc].use < 10)
                    sprintf(tmp, "         %d    %.1f    %s\n\n", cpuData[proc].processor,
                                                                  cpuData[proc].use, cpuData[proc].frequency);
                else
                    sprintf(tmp, "         %d    %.1f   %s\n\n", cpuData[proc].processor,
                                                                 cpuData[proc].use, cpuData[proc].frequency);
            }
            else  // final segment of master string
            {
                if (cpuData[proc].use < 10)
                    sprintf(tmp, "         %d    %.1f    %s\n", cpuData[proc].processor,
                                                                cpuData[proc].use, cpuData[proc].frequency);
                else
                    sprintf(tmp, "         %d    %.1f   %s\n", cpuData[proc].processor,
                                                               cpuData[proc].use, cpuData[proc].frequency);
            }
            strcat(cpu_detail, tmp);  // append segment
        }

    /* In all other cases, append uniform segments to master string. */
    else
        for(int proc = 0; proc < processors; proc++)
        {
            char tmp[CPU_SEG_CHAR];
            if (cpuData[proc].use < 10)
                sprintf(tmp, "      %d  %d    %.1f    %s\n", cpuData[proc].coreID, cpuData[proc].processor,
                                                             cpuData[proc].use, cpuData[proc].frequency);
            else
                sprintf(tmp, "      %d  %d    %.1f   %s\n", cpuData[proc].coreID, cpuData[proc].processor,
                                                            cpuData[proc].use, cpuData[proc].frequency);
            strcat(cpu_detail, tmp);
        }
}

// TRIGGERED BY "EXPAND" SLIDER TOGGLE; EXPAND/COLLAPSE DETAILED VIEW.
void switch_detail(GtkSwitch *swtch, gpointer user_data)
{
    if (!Expanded)
        Expanded = true;
    else
        Expanded = false;
}

/* TRIGGERED BY "%" CHECKBOX CLICK; SWITCH MEMORY DISPLAY
UNITS BETWEEN GIGABYTES AND PERCENTAGE OF TOTAL CAPACITY. */
void switch_mem_units(GtkCheckButton *chkbx)
{
    if (!Percentages)
    {
        strcpy(mem_unit, " %");
        Percentages = true;
    }
    else
    {
        strcpy(mem_unit, " GB"); 
        Percentages = false;
    }
}

//end
