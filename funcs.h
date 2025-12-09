#ifndef FUNCS_H
#define FUNCS_H

void menu_item_1(void);
void menu_item_2(void);
void menu_item_3(void);
void menu_item_4(void);

//menu 1
#define MAX_RESISTORS 5
#define MAX_CIRCUIT_LINES 10
#define MAX_LINE_LENGTH 100


extern char circuit_diagram[MAX_CIRCUIT_LINES][MAX_LINE_LENGTH];
extern int diagram_line_count;

// Function declarations
int is_positive(float val, const char *param_name);
float calc_series(float resistor[], int resistor_count);
float calc_parallel(float resistor[], int resistor_count);
float calc_mixed_connection(float resistors[], int resistor_count, int group_sizes[], int connection_types[], int group_count);
void draw_series_group(int group_size);
void draw_parallel_group(int group_size);
void draw_mixed_circuit(int group_sizes[], int connection_types[], int group_count);
void save_series_diagram(int group_size);
void save_parallel_diagram(int group_size);
void save_mixed_diagram(int group_sizes[], int connection_types[], int group_count);
void display_diagram(void);
void menu_item_1(void);
void handle_mixed_connection(float resistors[], int n);
float calc_mixed_resistance(float resistors[], int resistor_count, int group_sizes[], int connection_types[], int group_count);
void draw_mixed_circuit(int group_sizes[], int connection_types[], int group_count);
void save_mixed_diagram(int group_sizes[], int connection_types[], int group_count);
//menu 2
void unit_converter(void);

//menu 3
#define MAX_SIZE 10

// Matrix structure
typedef struct {
    double data[MAX_SIZE][MAX_SIZE];
    int rows;
    int cols;
} Matrix;

// Linear Algebra Library functions
void menu_item_3(void);
void linear_algebra_library(void);
void matrix_addition(void);
void matrix_multiplication(void);
void matrix_determinant(void);

// Utility functions for matrix operations
void input_matrix(Matrix *mat, const char *name);
void print_matrix(Matrix mat);
double calculate_determinant(Matrix mat);
int is_square_matrix(Matrix mat);
Matrix create_submatrix(Matrix mat, int exclude_row, int exclude_col);

// menu 4
// Thermodynamic constants
#define R_UNIVERSAL 8.314462618    // Universal gas constant [J/(mol·K)]
#define R_AIR 0.287               // Gas constant for air [kJ/(kg·K)]
#define CP_AIR 1.005              // Specific heat at constant pressure [kJ/(kg·K)]
#define CV_AIR 0.718              // Specific heat at constant volume [kJ/(kg·K)]
#define GAMMA_AIR 1.4             // Specific heat ratio for air

// Fluid types
typedef enum {
    FLUID_AIR,
    FLUID_WATER,
    FLUID_STEAM,
    FLUID_REFRIGERANT
} FluidType;

// Thermodynamic state point
typedef struct {
    double pressure;      // Pressure [kPa]
    double temperature;   // Temperature [K]
    double volume;       // Specific volume [m³/kg]
    double mass;         // Mass [kg]
    double enthalpy;     // Specific enthalpy [kJ/kg]
    double entropy;      // Specific entropy [kJ/(kg·K)]
    double internal_energy; // Specific internal energy [kJ/kg]
} StatePoint;

// Process types
typedef enum {
    ISOBARIC_PROCESS,    // Constant pressure
    ISOTHERMAL_PROCESS,  // Constant temperature
    ISOCHORIC_PROCESS,   // Constant volume
    ADIABATIC_PROCESS    // No heat transfer
} ProcessType;

// Main calculator function
void thermodynamic_properties_calculator(void);

// Core calculation modules
void advanced_ideal_gas_analyzer(void);
void enthalpy_entropy_analyzer(void);
void thermodynamic_cycle_analyzer(void);

// Utility functions
void input_thermodynamic_state(StatePoint* state, const char* label);
void print_comprehensive_analysis(StatePoint state);
double calculate_compressibility_factor(double P, double T, double v, FluidType fluid);
void perform_process_analysis(StatePoint initial, StatePoint final, ProcessType process);
#endif