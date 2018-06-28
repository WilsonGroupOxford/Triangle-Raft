#ifndef MX2_SIMULATION_H
#define MX2_SIMULATION_H

#include <iostream>
#include <string>
#include "logfile.h"
#include "monteCarlo.h"
#include "networkDerived.h"

using namespace std;

template <typename CrdT, typename NetT>
class Simulation {
    //controls network growth
private:
    //Input Variables
    //IO
    string prefixIn, prefixOut; //for read in/write out
    //Network Properties
    int nTargetRings, basicMinSize, basicMaxSize; //number of rings to build, basic ring size limits
    int dimensionality; //2D/3D
    string growthGeometry; //growth geometry
    //Monte Carlo
    MonteCarlo monteCarlo; //mc evaluator
    //Potential Model
    vector<double> potentialModel; //all k and r0 values for harmonic potential
    //Geometry Optimisation
    bool globalPreGO, globalPostGO; //points for global optimisation
    int goMaxIterations; //for descent algorithm
    double goLineSeachInc, goConvergence; //for descent algorithm

    //Additional variables
    NetT masterNetwork; //main global network for simulation

    //Key Methods
    void loadNetwork(Logfile &logfile);
    void growNetwork(Logfile &logfile);
    void writeNetwork(Logfile &logfile);

    //Aux Methods
    int selectActiveUnit(); //find unit to build new ring on
    vector<int> selectUnitPath(int activeUnit); //find path of units to build new ring on

public:
    //Constructors
    Simulation();
    Simulation(Logfile &logfile);

    //set input variables
    void setIO(string in, string out, Logfile &logfile);
    void setNP(int targRings, int basicMin, int basicMax, string geom, Logfile &logfile);
    void setMC(int seed, double temperature, Logfile &logfile);
    void setPM(double kMX, double r0MX, double kXX, double a0XX, double kMM, double a0MM, Logfile &logfile);
    void setGO(bool global0, bool global1, int it, double ls, double conv, Logfile &logfile);

    //methods
    void run(Logfile &logfile);

};

#include "simulation.tpp"

#endif //MX2_SIMULATION_H