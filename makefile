CXX = g++
CXXFLAGS = -fdiagnostics-color=always -mwindows -municode -Wall
LIBS = -lstrmiids -lOle32 -lOleAut32

AudioPlayBack: AudioPlayBack.o
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LIBS)

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $^

clean:
	del /f *.o *.exe