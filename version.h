#ifndef VERSION_H
#define VERSION_H

// #define VERSION "0.1.10 (build 2)"
// Allow zero as start period

// #define VERSION "0.1.11"
// Subtract loans from business sector balance

// #define VERSION "0.1.12"
// Allows chart profiles. Preliminary version only as no mechanism for removing
// profiles or for editing names.

// #define VERSION "0.1.12 (build 2)"
// Keeps the line edit in the control widget up to date when profile changes

// #define VERSION "0.1.12 (build 3)"
// Removed control widget area and added toolbar. Still no way of removing a
// profile. Must be done before going to next version.

// #define VERSION "0.1.12 (build 4)"
// Improved toolbar and added stats panel (incomplete). Still no way of
// removing a profile. Must be done before going to next version.

// #define VERSION "0.1.12 (build 5)"
// Added profile removal

// #define VERSION "0.1.13"
// Added statistics box

// #define VERSION "0.1.13 (build 2)"
// Improved toolbar behaviour

// #define VERSION "0.1.13 (build 3)"
// Improved docking

// #define VERSION "0.1.13 (build 4)"
// Changes stats after redrawing model

// #define VERSION "0.1.13 (build 5)"
// Changes stats after redrawing model

// #define VERSION "0.1.13 (build 6)"
// Added National Debt as percent of GDP
// Added Reset Colours action

// #define VERSION "0.1.14"
// Inequality and productivity statistics moved to status bar.

// #define VERSION "0.1.14 (build 2)"
// Corrected model setup wizard title.
// Removed 'National Debt as %GDP' as it doesn't make sense. What is really
// required is cumulative deficit as percent of cumulative consumption (GDP)
// over given period. Current 'Deficit as percent GDP' is its instantaneous
// ('marginal') value, and I'm not sure that a rollong value over an arbitrary
// period would be of much more value.

#define VERSION "0.1.15"
// Added investment, GDP and profit properties
// Added after reading Wray, Modern Money Theory, particuarly last chapter.
// The three properties are all defined as identities and do nor add any extra
// information -- except perhaps for investment, which was an internal variable
// used by firms in deciding how to dispose of excess funds.

#endif // VERSION_H
