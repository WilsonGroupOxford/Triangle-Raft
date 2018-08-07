#include "networkBase.h"


//##### NETWORK BASE #####

template <typename CrdT>
Network<CrdT>::Network() {
    //default constructor
    nAtoms=0;
    nUnits=0;
    nRings=0;
    energy=numeric_limits<double>::infinity();
    optIterations=-1;
    atoms.clear();
    units.clear();
    rings.clear();
    boundaryUnits.clear();
}

template <typename CrdT>
int Network<CrdT>::getNRings() {
    //return number of rings in network
    return nRings;
}

template <typename CrdT>
double Network<CrdT>::getEnergy() {
    //return energy of network
    return energy;
}

template <typename CrdT>
int Network<CrdT>::getIterations() {
    //return minimisation iterations
    return optIterations;
}

template <typename CrdT>
void Network<CrdT>::addAtom(Atom<CrdT> atom) {
    //add atom to network and update map
    atoms.push_back(atom);
    ++nAtoms;
}

template <typename CrdT>
void Network<CrdT>::addUnit(Unit unit) {
    //add unit to network and update map
    units.push_back(unit);
    ++nUnits;
}

template <typename CrdT>
void Network<CrdT>::addRing(Ring ring) {
    //add ring to network and update map
    rings.push_back(ring);
    ++nRings;
}

template <typename CrdT>
void Network<CrdT>::delAtom() {
    //delete atom from network
    atoms.pop_back();
    --nAtoms;
}

template <typename CrdT>
void Network<CrdT>::delUnit() {
    //delete unit from network
    units.pop_back();
    --nUnits;
}

template <typename CrdT>
void Network<CrdT>::delRing() {
    //delete ring from network
    rings.pop_back();
    --nRings;
}

template <typename CrdT>
int Network<CrdT>::addUnitAtomXCnx(int uId, int aId) {
    //add atom x id to unit
    int status=units[uId].atomsX.add(aId);
    return status;
}

template <typename CrdT>
int Network<CrdT>::addUnitRingCnx(int uId, int rId) {
    //add mutual unit-ring connection
    int uStatus=units[uId].rings.add(rId);
    int rStatus=rings[rId].units.add(uId);
    return uStatus+rStatus;
}

template <typename CrdT>
int Network<CrdT>::addUnitUnitCnx(int uId1, int uId2) {
    //add mutual unit-unit connection
    int status1=units[uId1].units.add(uId2);
    int status2=units[uId2].units.add(uId1);
    return status1+status2;
}

template <typename CrdT>
int Network<CrdT>::addRingRingCnx(int rId1, int rId2) {
    //add mutual unit-unit connection
    int status1=rings[rId1].rings.add(rId2);
    int status2=rings[rId2].rings.add(rId1);
    return status1+status2;
}

template <typename CrdT>
void Network<CrdT>::delUnitUnitCnx(int uId1, int uId2) {
    //del unit-unit connection
    units[uId1].units.del(uId2);
}

template <typename CrdT>
void Network<CrdT>::delUnitRingCnx(int uId, int rId) {
    //delete unit-ring connection
    units[uId].rings.del(rId);
}

template <typename CrdT>
void Network<CrdT>::delRingRingCnx(int rId1, int rId2) {
    //delete ring-ring connection
    rings[rId1].rings.del(rId2);
}

template <typename CrdT>
bool Network<CrdT>::checkActiveUnit(int &uId, int sumCheck) {
    //checks if a unit is active by summing coordination of associated x atoms
    double cndSum=0;
    for(int i=0; i<units[uId].atomsX.n; ++i){
        cndSum+=atoms[units[uId].atomsX.ids[i]].coordination;
    }
    if(cndSum<sumCheck) return true;
    else return false;
}

template <typename CrdT>
bool Network<CrdT>::checkEdgeUnit(int &uId, int ringCheck) {
    //checks if a unit is on edge by checking number of associated rings
    if(units[uId].rings.n<ringCheck) return true;
    else return false;
}

template <typename CrdT>
void Network<CrdT>::trialRing(int ringSize, vector<int> &unitPath, vector<double> &potentialModel) {
    //build a ring of a given size to a starting path
    //minimise and calculate energy
    //remove ring

    //build trial ring
    buildRing(ringSize, unitPath, potentialModel);

    //geometry optimise
    geometryOptimiseLocal(potentialModel);
//    geometryOptimiseGlobal(potentialModel);

    //pop trial ring
    popRing(ringSize, unitPath);

}

template <typename CrdT>
void Network<CrdT>::acceptRing(int ringSize, vector<int> &unitPath, vector<double> &potentialModel) {
    //build ring of given size to a starting path, minimise and calculate boundary

    buildRing(ringSize, unitPath, potentialModel);
    geometryOptimiseLocal(potentialModel);
//    geometryOptimiseGlobal(potentialModel);
    calculateBoundary();
}

template <typename CrdT>
void Network<CrdT>::calculateBoundary() {
    //find units on boundary of network
    //follow units on edge, flag when traversed
    //an edge triangle will have rings.n<3
    //an active triangle will have an atom with coordination<4
    //if active must be on edge

    boundaryUnits.clear();
    boundaryStatus.clear();

    //starting position as an active triangle on edge and arbitrary edge neighbour
    for(int i=0; i<nUnits; ++i){
        if(checkActiveUnit(i)){
            boundaryUnits.push_back(i);
            boundaryStatus.push_back(true);
            units[i].flag=true;
            //must have two neighbours on edge so pick first arbitrarily
            boundaryUnits.push_back(units[i].units.ids[0]);
            boundaryStatus.push_back(checkActiveUnit(boundaryUnits[1]));
            units[boundaryUnits.rbegin()[0]].flag=true;
            break;
        }
    }

    //trace perimeter back to start
    int id0=boundaryUnits.rbegin()[0], id1, id1a, id1b, id1c, id1d, id1e;
    bool active0=boundaryStatus.rbegin()[0], active1;
    bool complete=false;
    for(;;){
        if(id0==16){
            int a=0;
        }
        if(active0){//only one possiblity - pick non-flagged path
            for(int i=0; i<2; ++i){
                id1=units[id0].units.ids[i];
                if(!units[id1].flag) break;
                else if(id1==boundaryUnits[0] && boundaryUnits.size()>2) complete=true;
            }
        }
        else{
            //get ids of three connected units
            id1c=units[id0].units.ids[0];
            id1d=units[id0].units.ids[1];
            id1e=units[id0].units.ids[2];
            if(id1c==boundaryUnits[0] && boundaryUnits.size()>2) complete=true;
            if(id1d==boundaryUnits[0] && boundaryUnits.size()>2) complete=true;
            if(id1e==boundaryUnits[0] && boundaryUnits.size()>2) complete=true;
            //keep ids of those which are not flagged
            id1a=-1;
            id1b=-1;
            if(!units[id1c].flag){
                id1a=id1c;
                if(!units[id1d].flag) id1b=id1d;
                else if(!units[id1e].flag) id1b=id1e;
            }
            else if(!units[id1d].flag){
                id1a=id1d;
                if(!units[id1e].flag) id1b=id1e;
            }
            else if(!units[id1e].flag) id1a=id1e;
            //review possiblities
            if(id1b==-1){//only one non-flag option
                id1=id1a;
            }
            else{//two non-flag options, find number of edge options
                bool edgeA=checkEdgeUnit(id1a);
                bool edgeB=checkEdgeUnit(id1b);
                if(edgeA && !edgeB){//only a on edge
                    id1=id1a;
                }
                else if(!edgeA && edgeB){//only b on edge
                    id1=id1b;
                }
                else{//two edge options
                    //check number of associated rings - pick unit that has just one
                    if(units[id1a].rings.n==1 && units[id1b].rings.n>1) id1=id1a;
                    else if(units[id1a].rings.n>1 && units[id1b].rings.n==1) id1=id1b;
                    else{
                        //both have multiple rings - hardest case but also most rare
                        //compare which rings each unit belongs to, pick unit with just one match
                        int ring0i=units[id0].rings.ids[0];
                        int ring0ii=units[id0].rings.ids[1];
                        int ring1ai=units[id1a].rings.ids[0];
                        int ring1aii=units[id1a].rings.ids[1];
                        int ring1bi=units[id1b].rings.ids[0];
                        int ring1bii=units[id1b].rings.ids[1];
                        if(ring1ai!=ring0i && ring1ai!=ring0ii) id1=id1a;
                        else if(ring1aii!=ring0i && ring1aii!=ring0ii) id1=id1a;
                        else if(ring1bi!=ring0i && ring1bi!=ring0ii) id1=id1b;
                        else if(ring1bii!=ring0i && ring1bii!=ring0ii) id1=id1b;
                        else{
                            cout<<"Boundary failed"<<endl;
//                            consoleVector(boundaryUnits);
                            Logfile dump;
                            write("dump",false,dump);
                            exit(9);
                        }
                    }
                }
            }
        }
        if (complete) break;
        else {
            active1=checkActiveUnit(id1);
            boundaryUnits.push_back(id1);
            boundaryStatus.push_back(active1);
            units[id1].flag=true;
            id0 = id1;
            active0=active1;
        }
    }

    //convert boundary status values to atom which is active
    for(int i=0; i<boundaryStatus.size(); ++i){
        if(boundaryStatus[i]==0) boundaryStatus[i]=-1; //as 0 can be an id
        else{
            id0=boundaryUnits[i];
            for(int j=0; j<3; ++j){//loop over x atoms and find undercoordinated atom
                id1=units[id0].atomsX.ids[j];
                if(atoms[id1].coordination<4){
                    boundaryStatus[i]=id1;
                    break;
                }
            }
        }

    }

    //remove flag
    for(int i=0; i<boundaryUnits.size(); ++i) units[boundaryUnits[i]].flag=false;

//    consoleVector(boundaryUnits);
//    consoleVector(boundaryStatus);
}

template <typename CrdT>
vector<int> Network<CrdT>::getBoundarySection(int startId, bool direction) {
    //find section of unit boundary in given direction

    //find position of id in boundary
    int startPos = find(boundaryUnits.begin(), boundaryUnits.end(), startId) - boundaryUnits.begin();

    vector<int> section;
    section.clear();
    section.push_back(startId);
    int n=boundaryUnits.size();

    //search in one of two directions, loop over perimeter until find next active unit
    if(direction) {
        int j;
        for (int i = 1; i <= n; ++i) {
            j=(startPos + i) % n;
            section.push_back(boundaryUnits[j]);
            if(boundaryStatus[j]>=0) break;
        }
    }
    else{
        int j;
        for (int i = 1; i <= n; ++i) {
            j = (startPos + n - i) % n;
            section.push_back(boundaryUnits[j]);
            if(boundaryStatus[j]>=0) break;
        }
    }
    return section;
}

template <typename CrdT>
void Network<CrdT>::findLocalRegion(int &rId, int nFlexShells) {
    //find units around and included in a given ring, within a given number of connections, make map of corresponding local atoms
    //have flexible shells, then fixed shell

    localAtomMap.clear();
    globalAtomMap.clear();
    flexLocalUnits.clear();
    fixedLocalUnits.clear();
    fixedLocalAtoms.clear();

    //get units in ring
    vector<int> shell0(rings[rId].units.n), shell1;
    for(int i=0; i<rings[rId].units.n; ++i) shell0[i]=rings[rId].units.ids[i];
    for(int i=0; i<shell0.size(); ++i) flexLocalUnits.push_back(shell0[i]);

    //loop over flexible shells and get units, then get fixed shell
    for(int i=0; i<nFlexShells+1; ++i){
        shell1.clear();
        //find adjecent units to shell0
        for(int j=0; j<shell0.size(); ++j){
            for(int k=0; k<units[shell0[j]].units.n; ++k){
                shell1.push_back(units[shell0[j]].units.ids[k]);
            }
        }
        if(shell1.size()>0){
            //get unique units not in shell0
            sort(shell1.begin(), shell1.end());
            shell1.erase(unique(shell1.begin(), shell1.end()),shell1.end());
            for (int i=0; i <flexLocalUnits.size(); ++i) shell1.erase(remove(shell1.begin(), shell1.end(), flexLocalUnits[i]), shell1.end());
            //add to vector
            if(i!=nFlexShells){
                for(int i=0; i<shell1.size(); ++i) flexLocalUnits.push_back(shell1[i]);
            }
            else{
                for(int i=0; i<shell1.size(); ++i) fixedLocalUnits.push_back(shell1[i]);
            }
            shell0=shell1;
        }
        else break;
    }

    //make map of atoms to include in local region
    int m, x;
    nLocalAtoms=0;
    for(int i=0; i<flexLocalUnits.size(); ++i){
        m=units[flexLocalUnits[i]].atomM;
        if(localAtomMap.count(m)==0){
            localAtomMap[m]=nLocalAtoms;
            globalAtomMap[nLocalAtoms]=m;
            ++nLocalAtoms;
        }
        for(int j=0; j<units[flexLocalUnits[i]].atomsX.n; ++j){
            x=units[flexLocalUnits[i]].atomsX.ids[j];
            if(localAtomMap.count(x)==0){
                localAtomMap[x]=nLocalAtoms;
                globalAtomMap[nLocalAtoms]=x;
                ++nLocalAtoms;
            }
        }
    }
    for(int i=0; i<fixedLocalUnits.size(); ++i){
        m=units[fixedLocalUnits[i]].atomM;
        if(localAtomMap.count(m)==0){
            localAtomMap[m]=nLocalAtoms;
            globalAtomMap[nLocalAtoms]=m;
            fixedLocalAtoms.push_back(nLocalAtoms);
            ++nLocalAtoms;
        }
        for(int j=0; j<units[fixedLocalUnits[i]].atomsX.n; ++j){
            x=units[fixedLocalUnits[i]].atomsX.ids[j];
            if(localAtomMap.count(x)==0){
                localAtomMap[x]=nLocalAtoms;
                globalAtomMap[nLocalAtoms]=x;
                fixedLocalAtoms.push_back(nLocalAtoms);
                ++nLocalAtoms;
            }
        }
    }

//    cout<<"***"<<endl;
//    for(int i=0; i<fixedLocalAtoms.size(); ++i) {
//        cout<<globalAtomMap[fixedLocalAtoms[i]]<<endl;
//    }
//    cout<<"***"<<endl;
//
//    cout<<"-----"<<endl;
//    for(int i=0; i<nLocalAtoms;++i){
//        cout<<globalAtomMap[i]<<endl;
//    }
//    cout<<"-----"<<endl;


}

template <typename CrdT>
void Network<CrdT>::calculateRingStatistics() {
    //calculate ring statistics, ring statistics around each ring, and aboav-weaire analysis

    //calculate distribution of ring sizes and store unique ring sizes
    vector<int> ringSizes;
    for(int i=0; i<nRings; ++i) ringSizes.push_back(rings[i].units.n);
    DiscreteDistribution ringStats(ringSizes);
    ringStatistics=ringStats;

    //calculate distributions for each ring size (excluding edge rings)
    int ringRef;
    indRingStatistics.clear();
    for(int i=0; i<ringStatistics.n; ++i){//loop over ring sizes
        int s=ringStatistics.x[i];
        ringSizes.clear();
        for(int j=0; j<nRings; ++j){//get ring sizes around ring of given size
            if(rings[j].rings.full){//only include rings not on edge
                if(rings[j].units.n==s){
                    for(int k=0; k<rings[j].rings.n; ++k){
                        ringSizes.push_back(rings[rings[j].rings.ids[k]].units.n);
                    }
                }
            }
        }
        if(ringSizes.size()>0){
            DiscreteDistribution ringStats(ringSizes);
            indRingStatistics[s]=ringStats;
        }
    }

    //calculate aboav-weaire fit
    vector<double> x;
    vector<double> y;
    for(int i=0; i<ringStatistics.n; ++i){
        int s=ringStatistics.x[i];
        if(indRingStatistics.count(s)>0){
            x.push_back(ringStatistics.mean*(s-ringStatistics.mean));
            y.push_back(s*indRingStatistics[s].mean);
        }
    }
    aboavWeaireParameters=leastSquaresLinearRegression(x,y);
    aboavWeaireParameters[0]=1.0-aboavWeaireParameters[0]; //alpha
    aboavWeaireParameters[1]-=ringStatistics.mean*ringStatistics.mean; //mu
}

template <typename CrdT>
void Network<CrdT>::calculateBondDistributions() {
    //calculate bond length/angle distributions

    //M-X
    vector<double> bondLengths;
    bondLengths.clear();
    int m,x;
    CrdT crdM, crdX, crdMX;
    for(int i=0; i<nUnits; ++i){
        m=units[i].atomM;
        crdM=atoms[m].coordinate;
        for(int j=0; j<units[i].atomsX.n; ++j){
            x=units[i].atomsX.ids[j];
            crdX=atoms[x].coordinate;
            crdMX=crdM-crdX;
            bondLengths.push_back(crdMX.norm());
        }
    }
    ContinuousDistribution bondLenMX(bondLengths);
    bondLenDistMX=bondLenMX;

    //X-X
    bondLengths.clear();
    int x0, x1;
    CrdT crdX0, crdX1, crdXX;
    for(int i=0; i<nUnits; ++i){
        for(int j=0; j<units[i].atomsX.n-1; ++j){
            x0=units[i].atomsX.ids[j];
            crdX0=atoms[x0].coordinate;
            for(int k=j+1; k<units[i].atomsX.n; ++k){
                x1=units[i].atomsX.ids[k];
                crdX1=atoms[x1].coordinate;
                crdXX=crdX1-crdX0;
                bondLengths.push_back(crdXX.norm());
            }
        }
    }
    ContinuousDistribution bondLenXX(bondLengths);
    bondLenDistXX=bondLenXX;
}

template <typename CrdT>
void Network<CrdT>::write(string prefix, bool special, Logfile &logfile) {
    //write network and analysis to files
    writeNetwork(prefix,logfile);
    if(special) writeNetworkSpecial(prefix,logfile);
    writeAnalysis(prefix,logfile);
}

template <typename CrdT>
void Network<CrdT>::writeAnalysis(string prefix, Logfile &logfile) {
    //write analysis to file

    logfile.log("Writing network analysis","","",0,false);
    //set up analysis file
    string analysisFilename = prefix + "_analysis.out";
    ofstream analysisFile(analysisFilename,ios::in|ios::trunc);

    //overlap
    writeFileValue(analysisFile,"Valid geometry",true);
    writeFileValue(analysisFile,unitOverlap,true);

    //ring statistics
    writeFileValue(analysisFile,"p_n and <n>",true);
    analysisFile << fixed << showpoint << setprecision(1);
    vector<int> ringSizes=ringStatistics.getValues();
    writeFileVector(analysisFile,ringSizes);
    analysisFile << fixed << showpoint << setprecision(6);
    vector<double> data=ringStatistics.getProbabilities();
    data.push_back(ringStatistics.mean);
    writeFileVector(analysisFile,data);
    for(int i=0; i<ringSizes.size(); ++i){
        data.clear();
        int s=ringSizes[i];
        if(indRingStatistics.count(s)==0){
            for(int j=0; j<ringSizes.size()+1; ++j) data.push_back(0.0);
        }
        else{
            for(int j=0; j<ringSizes.size(); ++j){
                data.push_back(indRingStatistics.at(s).getProbability(ringSizes[j]));
            }
            data.push_back(indRingStatistics.at(s).mean);
        }
        writeFileVector(analysisFile,data);
    }
    logfile.log("Ring statistics written to: ",analysisFilename,"", 1, false);

    //ring correlations
    writeFileValue(analysisFile,"Aboav-Weaire alpha, mu and rsq",true);
    data=aboavWeaireParameters;
    writeFileVector(analysisFile,data);
    logfile.log("Aboav-Weaire parameters written to: ",analysisFilename,"", 1, false);

    logfile.log("Writing complete","","",0,true);
}
//template <typename CrdT>
//Network<CrdT>::
//template <typename CrdT>
//Network<CrdT>::
