#include <stdio.h>
#include "funcs.h"
#include <string.h>
#include <ctype.h>
#include <math.h>

#define RESISTOR "/\\/\\/\\"
#define WIRE "──"
#define NODE "●"
#define PARALLEL_VERT "│"       // Circuit diagram elements

#define MAX_CIRCUIT_LINES 10
#define MAX_LINE_LENGTH 100         // For circuit diagram storage array
#define MAX_GROUPS 5

char circuit_diagram[MAX_CIRCUIT_LINES][MAX_LINE_LENGTH];
int diagram_line_count = 0;

// Unit classification
typedef enum {
    UNIT_POWER,      // Power
    UNIT_FREQUENCY,  // Frequency  
    UNIT_TIME,       // Time
    UNIT_STORAGE,    // Storage
    UNIT_ANGLE,      // Angle
    UNIT_UNKNOWN     // Unknown
} UnitType;
// Unit information structure
typedef struct {
    const char* name;      // Unit name
    UnitType type;         // Unit type
    double to_base;        // Conversion factor to base unit
    const char* base_unit; // Base unit name
} UnitInfo;
// Unit database (easily extensible)
static UnitInfo unit_database[] = 
{
    // Power units (base: mW)
    {"dbm", UNIT_POWER, 1.0, "mw"},        // dBm directly corresponds to mW
    {"mw",  UNIT_POWER, 1.0, "mw"},        // Base unit
    {"w",   UNIT_POWER, 1000.0, "mw"},     // 1W = 1000mW
    
    // Frequency units (base: Hz)
    {"hz",  UNIT_FREQUENCY, 1.0, "hz"},    // Base unit
    {"khz", UNIT_FREQUENCY, 1000.0, "hz"}, // 1kHz = 1000Hz
    {"mhz", UNIT_FREQUENCY, 1e6, "hz"},    // 1MHz = 1,000,000Hz
    {"rpm", UNIT_FREQUENCY, 1.0/60.0, "hz"}, // 60RPM = 1Hz
    {"rad/s", UNIT_FREQUENCY, 1.0/(2 * 3.1415926535), "hz"}, // 2π rad/s = 1Hz
    
    // Time units (base: second)
    {"s",   UNIT_TIME, 1.0, "s"},          // Base unit
    {"ms",  UNIT_TIME, 0.001, "s"},        // 1ms = 0.001s
    {"us",  UNIT_TIME, 1e-6, "s"},         // 1μs = 0.000001s
    {"ns",  UNIT_TIME, 1e-9, "s"},         // 1ns = 0.000000001s
    
    // Storage units (base: byte)
    {"byte", UNIT_STORAGE, 1.0, "byte"},   // Base unit
    {"kb",   UNIT_STORAGE, 1024.0, "byte"}, // 1KB = 1024Bytes
    {"mb",   UNIT_STORAGE, 1024 * 1024.0, "byte"}, // 1MB = 1,048,576Bytes
    {"gb",   UNIT_STORAGE, 1024 * 1024 * 1024.0, "byte"}, // 1GB = 1,073,741,824Bytes
    
    // Angle units (base: radian)
    {"rad", UNIT_ANGLE, 1.0, "rad"},       // Base unit
    {"deg", UNIT_ANGLE, 3.1415926535/180.0, "rad"}, // 180° = π rad
    {"grad", UNIT_ANGLE, 3.1415926535/200.0, "rad"}, // 200grad = π rad
    
    {NULL, UNIT_UNKNOWN, 0, NULL}  // End marker
};

void menu_item_1(void) {
    //Input and verification of the number of resistors (2-5)
    printf("\n=====================================\n");
    printf("      Resistor Calculation Module\n");
    printf("=====================================\n");
    int n;
    printf("Please enter the number of resistors you want in the calculation, and it should between 2 and 5.\n");
    scanf("%d", &n);
    int c;
    while ((c = getchar()) != '\n' && c != EOF); //Clear buffer
    while(n<2||n>5)
    {
        printf("You have enter an invalid number, please enter again, it should between 2 and 5.\n");
        scanf("%d",&n);
        while ((c = getchar()) != '\n' && c != EOF);
    }
    
    // 2. Resistor Value Input and Positive Number Validation
    float r[5];
    char *o[5] = 
    {
    "first",
    "second",
    "third", 
    "fourth",
    "fifth"
    };
    for(int i=0;i<n;i++)
    {
        do
        {
            printf("Please enter the resistance of the %s resistor: ",o[i]);
            scanf("%f",&r[i]);
            while ((c = getchar()) != '\n' && c != EOF);
        }while(!is_positive(r[i],"the resistance"));
    }
    
    // 3. Selection of connection method and total resistance calculation
    int connection_method;
    float total_resistance;
    int group_number;
    int k=0;
    do
    {
        printf("\nPlease select the connection method of the resistors:\n");
        printf("\nMethod 1: Completely in Series.\nMethod 2: Completely in Parallel.\nMethod 3. Mixed Connection.\n");
        scanf("%d",&connection_method);
        while ((c = getchar()) != '\n' && c != EOF);
        switch(connection_method)
        {
            case 1:
                total_resistance=calc_series(r,n);
                printf("\nThe circuit diagram is shown below:\n\n");
                draw_series_group(n);
                printf("\nThe total resistance is: %fohms\n",total_resistance);
                break;
            case 2:
                total_resistance=calc_parallel(r,n);
                printf("\nThe circuit diagram is shown below:\n\n");
                draw_parallel_group(n);
                printf("\nThe total resistance is: %fohms\n",total_resistance);
                break;
            case 3:
            {
                if (n < 2) 
                {
                    printf("Mixed connection requires at least 2 resistors!\n");
                } 
                else 
                {
                    handle_mixed_connection(r, n);
                }
                break;
            }
            default:
            printf("Invalid connection method! Please enter 1 or 2.\n\n");
            break;
        }
    }
    while(connection_method<1||connection_method>3);
}
void handle_mixed_connection(float resistors[], int n)
{
    printf("\n=== Mixed Connection Configuration ===\n");
    
    int group_count;
    printf("How many groups do you want to create? (1-%d): ", (n < 3) ? n : 3);
    scanf("%d", &group_count);
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    while(group_count < 1 || group_count > n || group_count > 3) 
    {
        printf("Invalid! Please enter 1-%d: ", (n < 3) ? n : 3);
        scanf("%d", &group_count);
        while ((c = getchar()) != '\n' && c != EOF);
    }
    
    int group_sizes[MAX_GROUPS] = {0};
    int connection_types[MAX_GROUPS] = {0};
    int resistors_assigned = 0;
    
    printf("\nConfigure each group:\n");
    for(int i = 0; i < group_count; i++) 
    {
        int max_available = n - resistors_assigned-1;
        int min_required = (group_count - i - 1 > 0) ? 1 : (n - resistors_assigned);
        
        if (i == group_count - 1) 
        {
            group_sizes[i] = n - resistors_assigned;
            printf("Group %d will contain the remaining %d resistor(s)\n", i+1, group_sizes[i]);
        } 
        else 
        {
            printf("Enter number of resistors for Group %d (available: %d, minimum: %d): ", 
                   i+1, max_available, min_required);
            scanf("%d", &group_sizes[i]);
            while ((c = getchar()) != '\n' && c != EOF);
            
            while(group_sizes[i] < 1 || group_sizes[i] > max_available || 
                  resistors_assigned + group_sizes[i] + (group_count - i - 1) > n) 
                  {
                printf("Invalid! Enter %d-%d: ", min_required, max_available - (group_count - i - 1));
                scanf("%d", &group_sizes[i]);
                while ((c = getchar()) != '\n' && c != EOF);
            }
        }
        
        printf("Select connection type for Group %d (1=Series, 2=Parallel): ", i+1);
        scanf("%d", &connection_types[i]);
        while ((c = getchar()) != '\n' && c != EOF);
        
        while(connection_types[i] != 1 && connection_types[i] != 2) 
        {
            printf("Invalid! Enter 1 or 2: ");
            scanf("%d", &connection_types[i]);
            while ((c = getchar()) != '\n' && c != EOF);
        }
        
        resistors_assigned += group_sizes[i];
        printf("Group %d configured: %d resistor(s) in %s\n\n", 
               i+1, group_sizes[i], (connection_types[i] == 1) ? "series" : "parallel");
    }
    
    float total_resistance = calc_mixed_resistance(resistors, n, group_sizes, connection_types, group_count);
    
    printf("\n=== Mixed Connection Results ===\n");
    printf("Resistor values: ");
    for(int i = 0; i < n; i++) 
    {
        printf("%.1fΩ ", resistors[i]);
    }
    printf("\n\n");
    
    printf("Group configuration:\n");
    int resistor_index = 0;
    for(int i = 0; i < group_count; i++) 
    {
        printf("Group %d: ", i+1);
        for(int j = 0; j < group_sizes[i]; j++)
        {
            printf("%.1fΩ", resistors[resistor_index + j]);
            if(j < group_sizes[i] - 1) printf(" + ");
        }
        printf(" (%s)\n", (connection_types[i] == 1) ? "series" : "parallel");
        resistor_index += group_sizes[i];
    }
    
    printf("\nGroups connected in series.\n");
    printf("The circuit diagram is shown below:\n\n");
    draw_mixed_circuit(group_sizes, connection_types, group_count);
    printf("\nTotal Resistance: %.4f ohms\n", total_resistance);
}

float calc_mixed_resistance(float resistors[], int resistor_count, int group_sizes[], int connection_types[], int group_count) 
{
    float group_resistances[MAX_GROUPS];
    int resistor_index = 0;
    
    for(int i = 0; i < group_count; i++) 
    {
        if(connection_types[i] == 1) {
            group_resistances[i] = 0;
            for(int j = 0; j < group_sizes[i]; j++) 
            {
                group_resistances[i] += resistors[resistor_index++];
            }
        } else {
            float reciprocal_sum = 0;
            for(int j = 0; j < group_sizes[i]; j++) 
            {
                reciprocal_sum += 1.0 / resistors[resistor_index++];
            }
            group_resistances[i] = 1.0 / reciprocal_sum;
        }
    }
    
    float total_resistance = 0;
    for(int i = 0; i < group_count; i++) 
    {
        total_resistance += group_resistances[i];
    }
    return total_resistance;
}

void draw_mixed_circuit(int group_sizes[], int connection_types[], int group_count) {
    save_mixed_diagram(group_sizes, connection_types, group_count);
    display_diagram();
}

void save_mixed_diagram(int group_sizes[], int connection_types[], int group_count) {
    diagram_line_count = 0;

    int parallel_group_idx = -1;
    int series_before = 0;
    
    for(int i = 0; i < group_count; i++) 
    {
        if(connection_types[i] == 2) 
        {
            parallel_group_idx = i;
            break;
        }
        series_before += group_sizes[i];
    }
    
    if(parallel_group_idx == -1) 
    {
        int total_resistors = 0;
        for(int i = 0; i < group_count; i++) {
            total_resistors += group_sizes[i];
        }
        save_series_diagram(total_resistors);
        return;
    }
    
    int parallel_size = group_sizes[parallel_group_idx];

    int spaces_before = 8 * series_before;
    char space_str[100] = "";
    for(int i = 0; i < spaces_before; i++) 
    {
        strcat(space_str, " ");
    }
    
    if(parallel_size == 2)
    {
        char line[MAX_LINE_LENGTH];

        strcpy(line, space_str);
        strcat(line, "   ");
        strcat(line, WIRE);
        strcat(line, RESISTOR);
        strcat(line, WIRE);
        strcat(line, "   ");
        strcpy(circuit_diagram[diagram_line_count++], line);

        strcpy(line, space_str);
        strcat(line, "  ");
        strcat(line, PARALLEL_VERT);
        strcat(line, "          ");
        strcat(line, PARALLEL_VERT);
        strcpy(circuit_diagram[diagram_line_count++], line);

        strcpy(line, "");
        for(int i = 0; i < parallel_group_idx; i++) 
        {
            for(int j = 0; j < group_sizes[i]; j++) 
            {
                strcat(line, WIRE);
                strcat(line, RESISTOR);
            }
        }
        strcat(line, WIRE);
        strcat(line, NODE);
        strcat(line, WIRE);
        strcat(line, RESISTOR);
        strcat(line, WIRE);
        strcat(line, NODE);
        strcat(line, WIRE);

        for(int i = parallel_group_idx + 1; i < group_count; i++) 
        {
            for(int j = 0; j < group_sizes[i]; j++) 
            {
                strcat(line, WIRE);
                strcat(line, RESISTOR);
            }
        }
        strcat(line, WIRE);
        strcpy(circuit_diagram[diagram_line_count++], line);

        strcpy(line, space_str);
        strcat(line, "  ");
        strcat(line, PARALLEL_VERT);
        strcat(line, "          ");
        strcat(line, PARALLEL_VERT);
        strcpy(circuit_diagram[diagram_line_count++], line);

        strcpy(line, space_str);
        strcat(line, "   ");
        strcat(line, WIRE);
        strcat(line, RESISTOR);
        strcat(line, WIRE);
        strcat(line, "   ");
        strcpy(circuit_diagram[diagram_line_count++], line);
    }
    else if(parallel_size == 3) 
    {
        char line[MAX_LINE_LENGTH];

        strcpy(line, space_str);
        strcat(line, "   ");
        strcat(line, WIRE);
        strcat(line, RESISTOR);
        strcat(line, WIRE);
        strcat(line, "   ");
        strcpy(circuit_diagram[diagram_line_count++], line);

        strcpy(line, space_str);
        strcat(line, "  ");
        strcat(line, PARALLEL_VERT);
        strcat(line, "          ");
        strcat(line, PARALLEL_VERT);
        strcpy(circuit_diagram[diagram_line_count++], line);

        strcpy(line, "");
        for(int i = 0; i < parallel_group_idx; i++) {
            for(int j = 0; j < group_sizes[i]; j++) {
                strcat(line, WIRE);
                strcat(line, RESISTOR);
            }
        }
        strcat(line, WIRE);
        strcat(line, NODE);
        strcat(line, WIRE);
        strcat(line, RESISTOR);
        strcat(line, WIRE);
        strcat(line, NODE);
        
        for(int i = parallel_group_idx + 1; i < group_count; i++) {
            for(int j = 0; j < group_sizes[i]; j++) {
                strcat(line, WIRE);
                strcat(line, RESISTOR);
            }
        }
        strcat(line, WIRE);
        strcpy(circuit_diagram[diagram_line_count++], line);

        strcpy(line, space_str);
        strcat(line, "  ");
        strcat(line, PARALLEL_VERT);
        strcat(line, "          ");
        strcat(line, PARALLEL_VERT);
        strcpy(circuit_diagram[diagram_line_count++], line);
        
        strcpy(line, space_str);
        strcat(line, "   ");
        strcat(line, WIRE);
        strcat(line, RESISTOR);
        strcat(line, WIRE);
        strcat(line, "   ");
        strcpy(circuit_diagram[diagram_line_count++], line);
    }
    else 
    {
        char line[MAX_LINE_LENGTH];

        strcpy(line, space_str);
        strcat(line, "   ");
        strcat(line, WIRE);
        strcat(line, RESISTOR);
        strcat(line, WIRE);
        strcat(line, "   ");
        strcpy(circuit_diagram[diagram_line_count++], line);

        strcpy(line, space_str);
        strcat(line, "  ");
        strcat(line, PARALLEL_VERT);
        strcat(line, "          ");
        strcat(line, PARALLEL_VERT);
        strcpy(circuit_diagram[diagram_line_count++], line);

        strcpy(line, space_str);
        strcat(line, "   ");
        strcat(line, WIRE);
        strcat(line, RESISTOR);
        strcat(line, WIRE);
        strcat(line, "   ");
        strcpy(circuit_diagram[diagram_line_count++], line);

        strcpy(line, space_str);
        strcat(line, "  ");
        strcat(line, PARALLEL_VERT);
        strcat(line, "          ");
        strcat(line, PARALLEL_VERT);
        strcpy(circuit_diagram[diagram_line_count++], line);
        
        strcpy(line, "");
        for(int i = 0; i < parallel_group_idx; i++) {
            for(int j = 0; j < group_sizes[i]; j++) {
                strcat(line, WIRE);
                strcat(line, RESISTOR);
            }
        }
        strcat(line, WIRE);
        strcat(line, NODE);
        strcat(line, WIRE);
        strcat(line, RESISTOR);
        strcat(line, WIRE);
        strcat(line, NODE);
        
        for(int i = parallel_group_idx + 1; i < group_count; i++) {
            for(int j = 0; j < group_sizes[i]; j++) {
                strcat(line, WIRE);
                strcat(line, RESISTOR);
            }
        }
        strcat(line, WIRE);
        strcpy(circuit_diagram[diagram_line_count++], line);
        
        strcpy(line, space_str);
        strcat(line, "  ");
        strcat(line, PARALLEL_VERT);
        strcat(line, "          ");
        strcat(line, PARALLEL_VERT);
        strcpy(circuit_diagram[diagram_line_count++], line);
        
        strcpy(line, space_str);
        strcat(line, "   ");
        strcat(line, WIRE);
        strcat(line, RESISTOR);
        strcat(line, WIRE);
        strcat(line, "   ");
        strcpy(circuit_diagram[diagram_line_count++], line);
        
        strcpy(line, space_str);
        strcat(line, "  ");
        strcat(line, PARALLEL_VERT);
        strcat(line, "          ");
        strcat(line, PARALLEL_VERT);
        strcpy(circuit_diagram[diagram_line_count++], line);
        
        strcpy(line, space_str);
        strcat(line, "   ");
        strcat(line, WIRE);
        strcat(line, RESISTOR);
        strcat(line, WIRE);
        strcat(line, "   ");
        strcpy(circuit_diagram[diagram_line_count++], line);
    }
}

int is_positive(float val, const char *param_name) 
{
    if (val <= 0) 
    {
        printf("error：%s should greater than 0！\n", param_name);  
        return 0;  
    }
    return 1;  
}

float calc_series(float resistors[], int resistor_count) 
{
    float total = 0.0;
    //Iterate through all resistors and sum up their resistances (the total resistance in series = the sum of each resistor's resistance)
    for (int i = 0; i < resistor_count; i++) {
        total += resistors[i];
    }
    return total;
}

float calc_parallel(float resistors[], int resistor_count) 
{
    float reciprocal_sum = 0.0;
    //Iterate through all resistors and sum the reciprocals of each resistor (the reciprocal of the total parallel resistance equals the sum of the reciprocals of each resistor)
    for (int i = 0; i < resistor_count; i++) {
        reciprocal_sum += 1.0 / resistors[i];
    }
    return 1.0 / reciprocal_sum;
}

// Draw a single group circuit (parameters: number of resistors in the group, connection mode, group serial number)
void draw_group(int group_size, int conn_type, int group_idx) 
{
    printf("\nResistor Group %d:\n", group_idx);
    if (conn_type == 1) 
    {  // series
        draw_series_group(group_size);
    } 
    else if (conn_type == 2) 
    {  // parallel
        draw_parallel_group(group_size);
    }
    printf("\n");
}

// Draw a single group circuit
void draw_series_group(int group_size) 
{
    save_series_diagram(group_size);
    display_diagram();
}

// Draw a parallel circuit group
void draw_parallel_group(int group_size) 
{
    save_parallel_diagram(group_size);
    display_diagram();
}

void display_diagram(void) // To display each line of circuit_diagram[]
{
    for (int i = 0; i < diagram_line_count; i++) 
    {
        printf("%s\n", circuit_diagram[i]);
    }
}

void save_series_diagram(int group_size) // To save the diagram into circuit_diagram[]
{
    diagram_line_count = 0;
    char line[MAX_LINE_LENGTH];
    snprintf(line, MAX_LINE_LENGTH, "%s", WIRE);
    
    for (int i = 0; i < group_size; i++) 
    {
        strcat(line, RESISTOR);
        strcat(line, WIRE);
    }
    strcpy(circuit_diagram[diagram_line_count++], line);
}

void save_parallel_diagram(int group_size) // To save the diagram into circuit_diagram[]
{
    diagram_line_count = 0;
    switch(group_size) 
    {
        case 2:
        {
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"   %s%s%s   ", WIRE, RESISTOR, WIRE);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"  %s          %s",PARALLEL_VERT,PARALLEL_VERT);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH, "%s%s          %s%s", WIRE, NODE, NODE, WIRE);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"  %s          %s", PARALLEL_VERT, PARALLEL_VERT);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"   %s%s%s   ", WIRE, RESISTOR, WIRE);
            break;
        }
        case 3:
        {
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"   %s%s%s   ", WIRE, RESISTOR, WIRE);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"  %s          %s",PARALLEL_VERT,PARALLEL_VERT);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"%s%s%s%s%s%s%s", WIRE, NODE, WIRE, RESISTOR, WIRE, NODE, WIRE);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"  %s          %s",PARALLEL_VERT,PARALLEL_VERT);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"   %s%s%s   ", WIRE, RESISTOR, WIRE);
            break;
        }
        case 4:
        {
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"   %s%s%s   ", WIRE, RESISTOR, WIRE);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"  %s          %s",PARALLEL_VERT,PARALLEL_VERT);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"   %s%s%s   ", WIRE, RESISTOR, WIRE);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"  %s          %s",PARALLEL_VERT,PARALLEL_VERT);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH, "%s%s          %s%s", WIRE, NODE, NODE, WIRE);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"  %s          %s",PARALLEL_VERT,PARALLEL_VERT);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"   %s%s%s   ", WIRE, RESISTOR, WIRE);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"  %s          %s",PARALLEL_VERT,PARALLEL_VERT);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"   %s%s%s   ", WIRE, RESISTOR, WIRE);
            break;
        }
        case 5:
        {
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"   %s%s%s   ", WIRE, RESISTOR, WIRE);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"  %s          %s",PARALLEL_VERT,PARALLEL_VERT);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"   %s%s%s   ", WIRE, RESISTOR, WIRE);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"  %s          %s",PARALLEL_VERT,PARALLEL_VERT);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"%s%s%s%s%s%s%s", WIRE, NODE, WIRE, RESISTOR, WIRE, NODE, WIRE);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"  %s          %s",PARALLEL_VERT,PARALLEL_VERT);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"   %s%s%s   ", WIRE, RESISTOR, WIRE);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"  %s          %s",PARALLEL_VERT,PARALLEL_VERT);
            snprintf(circuit_diagram[diagram_line_count++], MAX_LINE_LENGTH,"   %s%s%s   ", WIRE, RESISTOR, WIRE);
            break;
        }
    }
}
// End of menu 1

void menu_item_2(void) 
{
    unit_converter(); 
}

// Find unit information
const UnitInfo* find_unit_info(const char* unit_name) {
    char lower_unit[20];
    strcpy(lower_unit, unit_name);
    for (char *p = lower_unit; *p; p++) *p = tolower(*p);
    
    for (int i = 0; unit_database[i].name != NULL; i++) {
        if (strcmp(lower_unit, unit_database[i].name) == 0) {
            return &unit_database[i];
        }
    }
    return NULL;
}

// Generic conversion function
double convert_units(double value, const char* from_unit, const char* to_unit) {
    const UnitInfo* from_info = find_unit_info(from_unit);
    const UnitInfo* to_info = find_unit_info(to_unit);
    
    if (!from_info || !to_info) {
        return NAN;  // Unit not found
    }
    
    if (from_info->type != to_info->type) {
        return NAN;  // Unit type mismatch
    }
    
    // Generic conversion formula: value * (from→base) / (to→base)
    double value_in_base = value * from_info->to_base;
    return value_in_base / to_info->to_base;
}

// Get conversion explanation
void get_conversion_explanation(const char* from_unit, const char* to_unit, char* explanation) {
    const UnitInfo* from_info = find_unit_info(from_unit);
    const UnitInfo* to_info = find_unit_info(to_unit);
    
    if (!from_info || !to_info || from_info->type != to_info->type) {
        strcpy(explanation, "Unsupported conversion");
        return;
    }
    
    // Generate explanation text
    if (from_info->to_base == 1.0) {
        // Converting from base unit
        sprintf(explanation, "1 %s = %.6g %s", 
                to_info->base_unit, to_info->to_base, to_info->name);
    } else if (to_info->to_base == 1.0) {
        // Converting to base unit
        sprintf(explanation, "1 %s = %.6g %s", 
                from_info->name, 1.0/from_info->to_base, from_info->base_unit);
    } else {
        // Cross-unit conversion
        sprintf(explanation, "1 %s = %.6g %s", 
                from_info->name, to_info->to_base/from_info->to_base, to_info->name);
    }
}

void unit_converter(void) {
    char input[100];
    
    printf("\n=====================================\n");
    printf("        Unit Converter\n");
    printf("=====================================\n");
    printf("Supported conversion types:\n");
    
    // Dynamically display supported units (generated from database)
    UnitType current_type = UNIT_UNKNOWN;
    for (int i = 0; unit_database[i].name != NULL; i++) {
        if (unit_database[i].type != current_type) {
            current_type = unit_database[i].type;
            printf("\n");
            switch (current_type) {
                case UNIT_POWER: printf("Power: "); break;
                case UNIT_FREQUENCY: printf("Frequency: "); break;
                case UNIT_TIME: printf("Time: "); break;
                case UNIT_STORAGE: printf("Storage: "); break;
                case UNIT_ANGLE: printf("Angle: "); break;
                default: break;
            }
        }
        printf("%s ", unit_database[i].name);
    }
    
    printf("\n\nInput format: value unit to target_unit");
    printf("\nExample: 1 MB to Byte, 10 dBm to mW, 60 RPM to Hz");
    printf("\nEnter 'back' to return to main menu");
    printf("\n=====================================\n");
    
    while (1) {
        printf("\n> ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "back") == 0) {
            printf("Return to main menu\n");
            break;
        }
        
        // Parse input
        double value;
        char from_unit[20], to_unit[20];
        
        if (sscanf(input, "%lf %s to %s", &value, from_unit, to_unit) == 3) {
            double result = convert_units(value, from_unit, to_unit);
            
            if (!isnan(result)) {
                char explanation[100];
                get_conversion_explanation(from_unit, to_unit, explanation);
                
                printf("Conversion result: %.6g %s\n", result, to_unit);
                printf("Conversion formula: %s\n", explanation);
            } else {
                printf("Error: Unsupported unit conversion\n");
                printf("Supported format: value unit to target_unit\n");
                printf("Example: 10 dBm to mW\n");
            }
        } else {
            printf("Error: Invalid input format\n");
            printf("Correct format: value unit to target_unit\n");
            printf("Example: 10 dBm to mW\n");
        }
    }
}

//End of menu 2

void menu_item_3(void)
{
    linear_algebra_library();
}

// Main linear algebra library interface
void linear_algebra_library(void) {
    int choice;
    
    printf("\n=====================================\n");
    printf("      Linear Algebra Calculator\n");
    printf("=====================================\n");
    
    while (1) {
        printf("\nSelect operation:\n");
        printf("1. Matrix Addition\n");
        printf("2. Matrix Multiplication\n");
        printf("3. Determinant Calculation\n");
        printf("4. Return to Main Menu\n");
        printf("Enter your choice (1-4): ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input! Please enter a number 1-4.\n");
            while (getchar() != '\n');
            continue;
        }
        
        switch (choice) {
            case 1:
                matrix_addition();
                break;
            case 2:
                matrix_multiplication();
                break;
            case 3:
                matrix_determinant();
                break;
            case 4:
                printf("Returning to main menu...\n");
                while (getchar() != '\n');
                return;
            default:
                printf("Invalid choice! Please select 1-4.\n");
        }
    }
}

// Input matrix from user
void input_matrix(Matrix *mat, const char *name) {
    printf("\n=== Input Matrix %s ===\n", name);
    
    printf("Enter number of rows (1-%d): ", MAX_SIZE);
    while (scanf("%d", &mat->rows) != 1 || mat->rows < 1 || mat->rows > MAX_SIZE) {
        printf("Invalid input! Enter number of rows (1-%d): ", MAX_SIZE);
        while (getchar() != '\n');
    }
    
    printf("Enter number of columns (1-%d): ", MAX_SIZE);
    while (scanf("%d", &mat->cols) != 1 || mat->cols < 1 || mat->cols > MAX_SIZE) {
        printf("Invalid input! Enter number of columns (1-%d): ", MAX_SIZE);
        while (getchar() != '\n');
    }
    
    printf("Enter matrix elements row by row:\n");
    for (int i = 0; i < mat->rows; i++) {
        printf("Row %d: ", i + 1);
        for (int j = 0; j < mat->cols; j++) {
            while (scanf("%lf", &mat->data[i][j]) != 1) {
                printf("Invalid input! Enter a valid number: ");
                while (getchar() != '\n');
            }
        }
    }
}

// Print matrix in formatted way
void print_matrix(Matrix mat) {
    printf("\nMatrix (%dx%d):\n", mat.rows, mat.cols);
    for (int i = 0; i < mat.rows; i++) {
        printf("| ");
        for (int j = 0; j < mat.cols; j++) {
            printf("%8.2f ", mat.data[i][j]);
        }
        printf("|\n");
    }
}

// Check if matrix is square
int is_square_matrix(Matrix mat) {
    return mat.rows == mat.cols;
}

// Create submatrix for determinant calculation
Matrix create_submatrix(Matrix mat, int exclude_row, int exclude_col) {
    Matrix submat;
    submat.rows = mat.rows - 1;
    submat.cols = mat.cols - 1;
    
    int sub_i = 0;
    for (int i = 0; i < mat.rows; i++) {
        if (i == exclude_row) continue;
        
        int sub_j = 0;
        for (int j = 0; j < mat.cols; j++) {
            if (j == exclude_col) continue;
            
            submat.data[sub_i][sub_j] = mat.data[i][j];
            sub_j++;
        }
        sub_i++;
    }
    
    return submat;
}

// Recursive determinant calculation
double calculate_determinant(Matrix mat) {
    // Base case: 1x1 matrix
    if (mat.rows == 1) {
        return mat.data[0][0];
    }
    
    // Base case: 2x2 matrix
    if (mat.rows == 2) {
        return mat.data[0][0] * mat.data[1][1] - mat.data[0][1] * mat.data[1][0];
    }
    
    double det = 0;
    
    // Laplace expansion along first row
    for (int col = 0; col < mat.cols; col++) {
        Matrix submat = create_submatrix(mat, 0, col);
        double cofactor = pow(-1, col) * mat.data[0][col] * calculate_determinant(submat);
        det += cofactor;
    }
    
    return det;
}

// Matrix Addition: C[i][j] = A[i][j] + B[i][j]
void matrix_addition(void) {
    printf("\n=== Matrix Addition ===\n");
    
    Matrix A, B, result;
    
    input_matrix(&A, "A");
    input_matrix(&B, "B");
    
    // Validate dimensions
    if (A.rows != B.rows || A.cols != B.cols) {
        printf("Error: Matrix dimensions must be equal for addition!\n");
        printf("Matrix A: %dx%d, Matrix B: %dx%d\n", A.rows, A.cols, B.rows, B.cols);
        return;
    }
    
    // Perform addition
    result.rows = A.rows;
    result.cols = A.cols;
    
    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < A.cols; j++) {
            result.data[i][j] = A.data[i][j] + B.data[i][j];
        }
    }
    
    // Display results
    printf("\nMatrix A:");
    print_matrix(A);
    
    printf("\nMatrix B:");
    print_matrix(B);
    
    printf("\nAddition Result (A + B):");
    print_matrix(result);
}

// Matrix Multiplication: C[i][j] = sum(A[i][k] * B[k][j])
void matrix_multiplication(void) {
    printf("\n=== Matrix Multiplication ===\n");
    
    Matrix A, B, result;
    
    input_matrix(&A, "A");
    input_matrix(&B, "B");
    
    // Validate dimensions
    if (A.cols != B.rows) {
        printf("Error: Incompatible dimensions for multiplication!\n");
        printf("Columns of A (%d) must equal rows of B (%d)\n", A.cols, B.rows);
        return;
    }
    
    // Perform multiplication
    result.rows = A.rows;
    result.cols = B.cols;
    
    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < B.cols; j++) {
            result.data[i][j] = 0;
            for (int k = 0; k < A.cols; k++) {
                result.data[i][j] += A.data[i][k] * B.data[k][j];
            }
        }
    }
    
    // Display results
    printf("\nMatrix A:");
    print_matrix(A);
    
    printf("\nMatrix B:");
    print_matrix(B);
    
    printf("\nMultiplication Result (A × B):");
    print_matrix(result);
}

// Determinant Calculation
void matrix_determinant(void) {
    printf("\n=== Determinant Calculation ===\n");
    
    Matrix mat;
    input_matrix(&mat, "");
    
    // Check if matrix is square
    if (!is_square_matrix(mat)) {
        printf("Error: Determinant is only defined for square matrices!\n");
        printf("Your matrix is %dx%d\n", mat.rows, mat.cols);
        return;
    }
    
    // Calculate determinant
    double det = calculate_determinant(mat);
    
    printf("\nInput Matrix:");
    print_matrix(mat);
    printf("\nDeterminant = %.2f\n", det);
}
//End of menu 3

void menu_item_4(void) 
{
    thermodynamic_properties_calculator();
}

void thermodynamic_properties_calculator(void) 
{
    int choice;
    
    printf("\n=====================================\n");
    printf("  Thermodynamic Properties Calculator\n");
    printf("=====================================\n");
    printf("Develop a tool that computes thermodynamic properties\n");
    printf("of gases or fluids, including temperature, pressure, and enthalpy.\n");
    printf("=====================================\n");
    
    while (1) 
    {
        printf("\n=== Advanced Analysis Modules ===\n");
        printf("1. Advanced Ideal Gas Law Analyzer\n");
        printf("2. Enthalpy & Entropy Deep Analysis\n");
        printf("3. Thermodynamic Cycle Analysis\n");
        printf("4. Return to Main Menu\n");
        printf("Select module (1-4): ");
        
        if (scanf("%d", &choice) != 1) 
        {
            printf("Invalid input! Please enter a number 1-4.\n");
            while (getchar() != '\n');
            continue;
        }
        
        switch (choice) 
        {
            case 1:
                advanced_ideal_gas_analyzer();
                break;
            case 2:
                enthalpy_entropy_analyzer();
                break;
            case 3:
                thermodynamic_cycle_analyzer();
                break;
            case 4:
                printf("Returning to main menu...\n");
                while (getchar() != '\n');
                return;
            default:
                printf("Invalid choice! Please select 1-4.\n");
        }
    }
}

// Advanced Ideal Gas Law Analyzer
void advanced_ideal_gas_analyzer(void) 
{
    printf("\n=== Advanced Ideal Gas Law Analyzer ===\n");
    printf("Deep analysis of gas behavior using Ideal Gas Law and real gas corrections\n\n");
    
    int analysis_type;
    printf("Select analysis type:\n");
    printf("1. Complete State Point Analysis\n");
    printf("2. Process Analysis (Multiple States)\n");
    printf("3. Real Gas Effects Analysis\n");
    printf("Enter choice (1-3): ");
    
    if (scanf("%d", &analysis_type) != 1) 
    {
        printf("Invalid input!\n");
        return;
    }
    
    StatePoint state;
    double mass, molar_mass;
    
    // Input basic properties
    printf("\nEnter gas properties:\n");
    printf("Pressure [kPa]: ");
    scanf("%lf", &state.pressure);
    printf("Temperature [K]: ");
    scanf("%lf", &state.temperature);
    printf("Mass [kg]: ");
    scanf("%lf", &mass);
    printf("Molar mass [g/mol] (Air=28.97, Water=18.02): ");
    scanf("%lf", &molar_mass);
    
    // Validate inputs
    if (state.pressure <= 0 || state.temperature <= 0 || mass <= 0 || molar_mass <= 0) 
    {
        printf("Error: All input values must be positive!\n");
        return;
    }
    
    // Calculate thermodynamic properties
    double P_pa = state.pressure * 1000; // Convert to Pa
    double n = mass / (molar_mass / 1000); // Number of moles
    state.volume = (n * R_UNIVERSAL * state.temperature) / P_pa / mass; // Specific volume
    
    // Determine specific heats based on fluid type
    double cp, cv;
    if (fabs(molar_mass - 28.97) < 0.1) 
    {
        cp = CP_AIR;
        cv = CV_AIR;
        printf("Fluid type: Air detected\n");
    } else if (fabs(molar_mass - 18.02) < 0.1) 
    {
        cp = 4.18; // Water specific heat
        cv = 4.18;
        printf("Fluid type: Water detected\n");
    } else 
    {
        cp = 1.0; // Default values
        cv = 0.718;
        printf("Fluid type: Custom (using default values)\n");
    }
    
    // Calculate energy properties
    state.internal_energy = cv * state.temperature;
    state.enthalpy = cp * state.temperature;
    state.entropy = cp * log(state.temperature / 298.15) - R_AIR * log(state.pressure / 101.325);
    state.mass = mass;
    
    // Display comprehensive analysis
    print_comprehensive_analysis(state);
    
    // Additional analysis based on user selection
    switch (analysis_type) 
    {
        case 2: 
        {
            // Process analysis
            StatePoint final_state;
            printf("\n=== PROCESS ANALYSIS ===\n");
            printf("Enter final state pressure [kPa]: ");
            scanf("%lf", &final_state.pressure);
            printf("Enter process type (1=Isobaric, 2=Isothermal, 3=Adiabatic): ");
            int process;
            scanf("%d", &process);
            perform_process_analysis(state, final_state, (ProcessType)process);
            break;
        }
        case 3: 
        {
            // Real gas effects analysis
            double Z = calculate_compressibility_factor(state.pressure, state.temperature, 
                                                       state.volume, FLUID_AIR);
            printf("\n=== REAL GAS EFFECTS ANALYSIS ===\n");
            printf("Compressibility Factor Z: %.4f\n", Z);
            if (Z < 0.95) {
                printf("Gas behavior: STRONG real gas effects (use real gas equation)\n");
            } else if (Z > 1.05) {
                printf("Gas behavior: Moderate real gas effects\n");
            } else {
                printf("Gas behavior: Near ideal gas (ideal gas law applicable)\n");
            }
            break;
        }
    }
}

// Enthalpy and Entropy Deep Analysis
void enthalpy_entropy_analyzer(void) 
{
    printf("\n=== Enthalpy & Entropy Deep Analysis ===\n");
    printf("Comprehensive energy and thermodynamic analysis\n\n");
    
    StatePoint state1, state2;
    double mass_flow;
    
    printf("=== INITIAL STATE ===\n");
    printf("Pressure P1 [kPa]: ");
    scanf("%lf", &state1.pressure);
    printf("Temperature T1 [K]: ");
    scanf("%lf", &state1.temperature);
    
    printf("\n=== FINAL STATE ===\n");
    printf("Pressure P2 [kPa]: ");
    scanf("%lf", &state2.pressure);
    printf("Temperature T2 [K]: ");
    scanf("%lf", &state2.temperature);
    
    printf("Mass flow rate [kg/s]: ");
    scanf("%lf", &mass_flow);
    
    // Calculate enthalpy and entropy changes
    double cp = CP_AIR; // Using air properties
    double delta_H = mass_flow * cp * (state2.temperature - state1.temperature);
    double delta_S = mass_flow * cp * log(state2.temperature / state1.temperature) 
                   - mass_flow * R_AIR * log(state2.pressure / state1.pressure);
    
    // Second law analysis
    double T0 = 298.15; // Reference temperature
    double exergy_destruction = fmax(0, T0 * delta_S);
    double work_potential = delta_H - exergy_destruction;
    double second_law_efficiency = (work_potential / fabs(delta_H)) * 100;
    
    printf("\n=== ENERGY ANALYSIS RESULTS ===\n");
    printf("Enthalpy Change: ΔH = %.3f kW\n", delta_H);
    printf("Entropy Change: ΔS = %.4f kW/K\n", delta_S);
    
    printf("\n=== SECOND LAW ANALYSIS ===\n");
    printf("Exergy Destruction: %.3f kW\n", exergy_destruction);
    printf("Maximum Work Potential: %.3f kW\n", work_potential);
    printf("Second Law Efficiency: %.1f%%\n", second_law_efficiency);
    
    // Process classification
    if (delta_S > 0.1) 
    {
        printf("Process classification: HIGHLY IRREVERSIBLE\n");
    } else if (delta_S < -0.1) 
    {
        printf("Process classification: Order increasing (requires work input)\n");
    } else 
    {
        printf("Process classification: Near reversible process\n");
    }
}

// Thermodynamic Cycle Analysis
void thermodynamic_cycle_analyzer(void) 
{
    printf("\n=== Thermodynamic Cycle Analysis ===\n");
    printf("Analysis of power cycles and refrigeration cycles\n\n");
    
    int cycle_type;
    printf("Select cycle type:\n");
    printf("1. Carnot Cycle (Theoretical Maximum Efficiency)\n");
    printf("2. Brayton Cycle (Gas Turbine)\n");
    printf("3. Rankine Cycle (Steam Power Plant)\n");
    printf("Enter choice (1-3): ");
    scanf("%d", &cycle_type);
    
    double efficiency, work_output, heat_input;
    
    switch (cycle_type) 
    {
        case 1: 
        {
            // Carnot Cycle
            double T_hot, T_cold;
            printf("Enter hot reservoir temperature [K]: ");
            scanf("%lf", &T_hot);
            printf("Enter cold reservoir temperature [K]: ");
            scanf("%lf", &T_cold);
            
            efficiency = 1 - T_cold / T_hot;
            printf("\n=== CARNOT CYCLE ANALYSIS ===\n");
            printf("Theoretical Maximum Efficiency: %.2f%%\n", efficiency * 100);
            printf("This represents the absolute maximum possible efficiency\n");
            break;
        }
            
        case 2: {
            // Brayton Cycle
            double pressure_ratio, T_max;
            printf("Enter compressor pressure ratio: ");
            scanf("%lf", &pressure_ratio);
            printf("Enter turbine inlet temperature [K]: ");
            scanf("%lf", &T_max);
            
            efficiency = 1 - 1 / pow(pressure_ratio, (GAMMA_AIR - 1) / GAMMA_AIR);
            printf("\n=== BRAYTON CYCLE ANALYSIS ===\n");
            printf("Thermal Efficiency: %.2f%%\n", efficiency * 100);
            printf("Typical for modern gas turbines: 35-45%%\n");
            break;
        }
            
        case 3: {
            // Rankine Cycle
            double P_high, P_low;
            printf("Enter boiler pressure [kPa]: ");
            scanf("%lf", &P_high);
            printf("Enter condenser pressure [kPa]: ");
            scanf("%lf", &P_low);
            
            efficiency = 0.35; // Typical value
            printf("\n=== RANKINE CYCLE ANALYSIS ===\n");
            printf("Estimated Efficiency: ~35%% (typical for steam cycles)\n");
            printf("Actual efficiency depends on turbine design and operating conditions\n");
            break;
        }
            
        default:
            printf("Invalid cycle type selection!\n");
            return;
    }
    
    printf("\n=== CYCLE PERFORMANCE SUMMARY ===\n");
    printf("Theoretical Efficiency: %.2f%%\n", efficiency * 100);
    printf("Practical Efficiency (estimated): %.2f%%\n", efficiency * 0.8 * 100);
    printf("Note: Real cycles have additional losses due to friction, heat loss, etc.\n");
}

// Utility function to print comprehensive analysis
void print_comprehensive_analysis(StatePoint state) {
    double n = state.mass / 0.02897; // Moles of air
    double total_volume = state.mass * state.volume;
    
    printf("\n=== COMPREHENSIVE GAS ANALYSIS ===\n");
    
    printf("Basic Properties:\n");
    printf("Pressure: %.2f kPa = %.2f bar\n", state.pressure, state.pressure / 100);
    printf("Temperature: %.2f K = %.2f °C\n", state.temperature, state.temperature - 273.15);
    printf("Specific Volume: %.6f m³/kg\n", state.volume);
    printf("Density: %.2f kg/m³\n", 1.0 / state.volume);
    
    printf("\nEnergy Properties:\n");
    printf("Specific Internal Energy: %.2f kJ/kg\n", state.internal_energy);
    printf("Specific Enthalpy: %.2f kJ/kg\n", state.enthalpy);
    printf("Specific Entropy: %.4f kJ/(kg·K)\n", state.entropy);
    
    printf("\nMolar Analysis:\n");
    printf("Number of moles: %.2f mol\n", n);
    printf("Total Volume: %.4f m³ = %.1f L\n", total_volume, total_volume * 1000);
    printf("Molar Volume: %.6f m³/mol\n", total_volume / n);
    
    // Compressibility factor analysis
    double Z = calculate_compressibility_factor(state.pressure, state.temperature, 
                                               state.volume, FLUID_AIR);
    printf("Compressibility Factor Z: %.4f\n", Z);
    
    // Gas behavior classification
    if (Z < 0.95) {
        printf("Gas behavior: STRONG real gas effects present\n");
    } else if (Z > 1.05) {
        printf("Gas behavior: Moderate real gas effects\n");
    } else {
        printf("Gas behavior: Near ideal gas conditions\n");
    }
}

// Calculate compressibility factor
double calculate_compressibility_factor(double P, double T, double v, FluidType fluid) {
    // Simplified compressibility factor calculation
    // Using corresponding states principle
    double Pc, Tc; // Critical parameters
    switch (fluid) {
        case FLUID_AIR:
            Pc = 3786; // kPa
            Tc = 132.5; // K
            break;
        case FLUID_WATER:
            Pc = 22064; // kPa
            Tc = 647.1; // K
            break;
        default:
            Pc = 4000; Tc = 150;
    }
    
    double Pr = P / Pc; // Reduced pressure
    double Tr = T / Tc; // Reduced temperature
    
    // Simple correlation for compressibility factor
    return 1.0 + 0.2 * Pr / Tr; // Approximate correlation
}

// Perform process analysis between two states
void perform_process_analysis(StatePoint initial, StatePoint final, ProcessType process) {
    double work = 0, heat = 0;
    const char* process_name[] = {"", "Isobaric", "Isothermal", "Adiabatic"};
    
    printf("\n=== %s PROCESS ANALYSIS ===\n", process_name[process]);
    
    switch (process) {
        case ISOBARIC_PROCESS:
            work = initial.pressure * (final.volume - initial.volume) * initial.mass;
            heat = initial.mass * CP_AIR * (final.temperature - initial.temperature);
            break;
            
        case ISOTHERMAL_PROCESS:
            work = initial.mass * R_AIR * initial.temperature * log(final.volume / initial.volume);
            heat = work;
            break;
            
        case ADIABATIC_PROCESS:
            work = -initial.mass * CV_AIR * (final.temperature - initial.temperature);
            heat = 0;
            break;
    }
    
    printf("Work done: %.2f kJ\n", work);
    printf("Heat transfer: %.2f kJ\n", heat);
    printf("Internal energy change: %.2f kJ\n", 
           initial.mass * CV_AIR * (final.temperature - initial.temperature));
}