
#define LINKAGE_14074 0
#define LINKAGE_14078 1
#define LINKAGE_14082 2
#define LINKAGE_14274 3
#define LINKAGE_14278 4
#define LINKAGE_14282 5
#define LINKAGE_14474 6
#define LINKAGE_14478 7
#define LINKAGE_14482 8

struct SuspensionLinkage {
    
    Graph1f compressionFactor;
    Graph1f ratio;

    float dogboneLength;
    float lowerLength;

    void initGraphs(){
        DataSet compressionFactorData = loadFileFloats("testdata.txt");
        compressionFactor.set(compressionFactorData);

        DataSet ratioData = loadFileFloats("testdata.txt");
        ratio.set(ratioData);

        std::cout << "Test: " << ratio.evaluate(9) << std::endl;
    }

    SuspensionLinkage(int linkage){
        if(linkage == LINKAGE_14074){
            dogboneLength = 140;
            lowerLength = 74;
        } else if(linkage == LINKAGE_14078){
            dogboneLength = 140;
            lowerLength = 78;
        } else if(linkage == LINKAGE_14082){
            dogboneLength = 140;
            lowerLength = 82;
        } else if(linkage == LINKAGE_14274){
            dogboneLength = 142;
            lowerLength = 74;
        } else if(linkage == LINKAGE_14278){
            dogboneLength = 142;
            lowerLength = 78;
        } else if(linkage == LINKAGE_14282){
            dogboneLength = 142;
            lowerLength = 82;
        } else if(linkage == LINKAGE_14474){
            dogboneLength = 144;
            lowerLength = 74;
        } else if(linkage == LINKAGE_14478){
            dogboneLength = 144;
            lowerLength = 78;
        } else if(linkage == LINKAGE_14482){
            dogboneLength = 144;
            lowerLength = 82;
        }

        initGraphs();
    }
};

struct SuspensionShock {
    
    float springRate;
    float preload;

    SuspensionShock(){
        springRate = 4.6 * 9.81;
        preload = 25;
    }
};

class Suspension {

public:
    SuspensionLinkage linkage = SuspensionLinkage(LINKAGE_14278);
    SuspensionShock shock = SuspensionShock();

    float calculateCompressionFactor(){
        return 5;
    }


};
