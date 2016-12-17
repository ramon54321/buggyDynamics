a:
	g++ -std=c++11 -I/usr/local/include -I./include -L/usr/local/lib -L./libs main.cpp -lode -lpthread -lglfw3 -lGLEW -lGL -lX11 -ldl -lXrandr -lXi -lXxf86vm -lXinerama -lXcursor