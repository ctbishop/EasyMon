// EasyMon Helper Functions.


#include "easymon.h"

// Function applies CSS from stylesheet via new provider.
void css_add_provider_with_styles(GtkCssProvider *provider, const char *stylesheet, GtkWidget *window)
{
    // Load CSS styles from file to the given provider.
    gtk_css_provider_load_from_file(provider, g_file_new_for_path(stylesheet));

    // Set provider to apply styles to application window.
    gtk_style_context_add_provider_for_display
    (
        gtk_widget_get_display(window),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

// Function hides normal content, replacing it with an error message.
void show_error_message(Main_Container *primaryBox)
{
    // Hide content.
    gtk_widget_set_visible(primaryBox->stack, FALSE);
    gtk_widget_set_visible(primaryBox->stackButtons, FALSE);

    // Unhide error message.
    gtk_widget_set_visible(primaryBox->errorMessage, TRUE);
}

// Function sets formatted text with markup to the title of the given expander.
void set_expander_title(GtkExpander *expander, const char *name, const char *format, double val, double factor, char *unit)
{
    char *str1 = g_strdup_printf("%.1f", factor * val);
    char *str2 = g_strdup_printf(format, name, str1, unit);
    g_free(str1);
    gtk_expander_set_label(expander, str2);
    g_free(str2);
}

// Function clears the given grid of all contents.
void grid_clear(GtkGrid *grid)
{
    // Iteratively remove first row of grid, until reaching NULL.
    while (gtk_grid_get_child_at(grid, 0, 0) != NULL)
    {
        gtk_grid_remove_row(grid, 0);
    }
}

// Function sets core ID or processor number to label at given position in the cpu expander.
void new_grid_label_int(GtkGrid *grid, int val, int row, int col, int row_span, int col_span)
{
    char *str = g_strdup_printf("%i", val);
    gtk_grid_attach(
        grid,
        gtk_label_new(str),
        row,
        col,
        row_span,
        col_span
    );
    g_free(str);
}

// Function sets usage percent to label at given position in the cpu expander.
void new_grid_label_float(GtkGrid *grid, float val, int row, int col, int row_span, int col_span)
{
    char *str = g_strdup_printf("%.1f", val);
    gtk_grid_attach
    (
        grid,
        gtk_label_new(str),
        row,
        col,
        row_span,
        col_span
    );
    g_free(str);
}

// Function sets formatted text to a label in the memory expander.
void set_mem_label(double val, GtkLabel *label, char *format, double factor)
{
    char *str = g_strdup_printf(format, factor * val);
    gtk_label_set_label(label, str);
    g_free(str);
}

/* Function sets cursor to index of first numeric character in the given buffer,
   returning false if buffer contains no numeric characters, otherwise true. */
bool advance_cursor_to_digit(char *buffer, size_t *cursor)
{
    int index = *cursor;
    while (!isdigit(buffer[index]))
    {
        if (buffer[index] == '\0')
        {
            return false;
        }
        index++;
    }
    *cursor = index;
    return true;
}

/* Function checks if given data indicates multiple threads per core, returning
   true if any instance is found, otherwise false. */
bool hyper_thread(CPUs cpuData[], int prc_count)
{
    // Check for inequality between coreID and processor number.
    for (int count = 0; count < prc_count; count++)
    {
        if (cpuData[count].coreID != cpuData[count].processor)
        {
            return true;
        }
    }
    return false;
}

// Function sorts given data in order of increasing core ID.
void sort_cpu_data(CPUs cpuData[], int prc_count)
{
    /* Iterate through all pairs of adjacent elements, swapping their positions
       if needed. Repeat process until a full pass is made without any swaps. */
    bool swap = true;
    while (swap)
    {
        swap = false;
        for (int first = 0, second = 1; second < prc_count; first++, second++)
        {
            if (cpuData[first].coreID > cpuData[second].coreID)
            {
                CPUs tmp = cpuData[first];
                cpuData[first] = cpuData[second];
                cpuData[second] = tmp;
                swap = true;
            }
        }
    }
}

