import os
os.system('echo "############################\n#    WiringPi\n############################"')
os.system('cd wiringPi/wiringPi && gcc -I. -c *.c')
os.system('echo "############################\n#    RaspiCamera\n############################"')
os.system('cd raspicam-0.1.6/src && g++ -I../dependencies/current/interface -I../dependencies/current -I. -c *.cpp private/*.cpp private_still/*.cpp -std=c++11')
os.system('echo "############################\n#    heading_err\n############################"')
os.system('g++ -Idarknet/src/ -Iraspicam-0.1.6/src -c heading_err.cpp darknet/obj/*.o')
os.system('echo "############################\n#    Final Assembly\n############################"')
os.system('g++ -pthread -o main_control heading_err.o darknet/obj/*.o raspicam-0.1.6/src/*.o wiringPi/wiringPi/*.o -lrt -lm -lltdl -lcrypt -lltdl -lopencv_imgcodecs -lopencv_core -lopencv_highgui -lopencv_imgproc -L/usr/local/include/opencv2 -lcuda -lcudart -L/usr/local/cuda-9.0/lib64')