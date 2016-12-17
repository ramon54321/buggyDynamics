
#include "spline.h"

struct ContactInfo {
	dContact contact;
	dJointFeedback feedback;
};

struct DistanceTracker {

    DistanceTracker() : distance(0), previousDistance(0){}

    float distance;
    float previousDistance;
};

float lerp(float a, float b, float f){
	return a + f * (b - a);
}

float invLerp(float a, float b, float v){
	return (v - a) / (b - a);
}

struct DataSet {
    std::vector<float> data;
    float min;
    float max;
};

class Graph1f {
public:
	//float* data;
	//int length;
    std::vector<float> data;
	float min, max;

/*
	Graph1f(float* dat, int len, float mi, float ma){
		data = dat;
		length = len;
		min = mi;
		max = ma;
	}

    Graph1f(){

    }

    void set(float* dataset){
        data = dataset + 3;
		length = *(dataset);
		min = *(dataset + 1);
		max = *(dataset + 2);
    }
*/
    Graph1f(){

    }

    Graph1f(DataSet dataSet){
        set(dataSet);
    }

    void set(DataSet dataSet){
        data = dataSet.data;
		min = dataSet.min;
		max = dataSet.max;
    }

	float evaluate(float x){
		if(x < min){
			return data[0];
		}

		if(x > max){
			return data.back();
		}

		float gapSize = max / (data.size()-1);
		int index = 1.0 / gapSize * x;
		int indexN = index+1;

		float overrun = x - (index * gapSize);
		float overrunFactor = invLerp(0, gapSize, overrun);

		float valIndex = data[index];
		float valIndexN = data[indexN];

		return lerp(valIndex, valIndexN, overrunFactor);
	}
};

DataSet loadFileFloats(char* filename){
    std::ifstream file;
    file.open(filename);
    DataSet dataSet;
    std::string line;
    int index = 0;
    if(file.is_open()){
        while(std::getline(file, line)){
            if(index == 0){
                dataSet.min = std::stof(line.c_str());
                index++;
                continue;
            }
            if(index == 1){
                dataSet.max = std::stof(line.c_str());
                index++;
                continue;
            }
            dataSet.data.push_back(std::stof(line.c_str()));
        }
        file.close();
    }
    return dataSet;
}

#define DRAW_METHOD_TRIANGLES 1
#define DRAW_METHOD_WIRES 2
#define DRAW_METHOD_POINTS 3


