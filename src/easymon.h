// Macros, Structure Definitions, and Function Prototypes for EasyMon Program.


#ifndef EASYMON_H
#define EASYMON_H

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GTK external library. Also includes glib.
#include <gtk/gtk.h>

// Maximum logical processors accomodated by program.
#define MAX_PROC 16

// Number of transient memory values displayed in application window.
#define MEM_VALS 4

// Buffer sizing macros - maximum size of lines read from respective files.
#define MAX_LINE_MEMINFO 50
#define MAX_LINE_STAT 100
#define MAX_LINE_CPUINFO 30

// String sizing macros - maximum size of values picked from cpuinfo and meminfo.
#define FRQ_CHAR 5
#define MEM_CHAR 10

// Structure for CPU data array.
typedef struct CPUs
{
    float use;
    int processor;
    char frequency[FRQ_CHAR];
    int coreID;
} CPUs;

// Direct children of highest level box in application window.
typedef struct Main_Container
{
    GtkWidget *stack;
    GtkWidget *stackButtons;
    GtkWidget *errorMessage;
} Main_Container;

// Objects needed to update transients in cpu display.
typedef struct CPU_Display
{
    GtkExpander *expander;
    GtkGrid *grid;
} CPU_Display;

// Objects needed to update transients in memory display.
typedef struct Mem_Display
{
    GtkExpander *expander;
    GtkLabel *buffers;
    GtkLabel *pageCache;
    GtkLabel *swapCache;
    GtkLabel *totalInstalled;
} Mem_Display;

// Function prototypes for easymon.c.
static void cleanup();
void setup_make_gui(GtkApplication *app, gpointer user_data);
bool startup_prepare_display();
gboolean timeout_update_display();
bool populate_cpu_use(CPUs cpuData[], double *cpuUse_aggregate, int *prc_count);
bool populate_cpu_frqs_ids(CPUs cpuData[], int *frq_count);
bool populate_mem_vals(double memVals[]);
static void on_expander_notify_expanded(GtkExpander *expander);
static void on_intervalDropdown_notify_selected(GtkDropDown *intervalDropdown);
static void on_unitsSwitch_state_set(GtkSwitch *unitsSwitch);
static void on_colorSwitch_state_set(GtkSwitch *colorSwitch);

// Function prototypes for helpers.c.
void css_add_provider_with_styles(GtkCssProvider *provider, const char *stylesheet, GtkWidget *window);
void show_error_message(Main_Container *primaryBox);
void grid_clear(GtkGrid *grid);
void set_expander_title(GtkExpander *expander, const char *name, const char *format, double val, double factor, char *unit);
void new_grid_label_int(GtkGrid *grid, int val, int row, int col, int row_span, int col_span);
void new_grid_label_float(GtkGrid *grid, float val, int row, int col, int row_span, int col_span);
void set_mem_label(double val, GtkLabel *label, char *format, double factor);
bool advance_cursor_to_digit(char *buffer, size_t *cursor);
bool hyper_thread(CPUs cpuData[], int prc_count);
void sort_cpu_data(CPUs cpuData[], int prc_count);

#endif

