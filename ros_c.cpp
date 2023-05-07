#include "ros/ros.h"
#include "std_msgs/Float32MultiArray.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cmath>

// deflaut setting
float offset_ch1[5] ;
float offset_ch2[5] ;
float offset_ch3[5] ;
float offset_ch4[5] ;
bool flag = false;

float strain_ch1[5] ;
float strain_ch2[5] ;
float strain_ch3[5] ;
float strain_ch4[5] ;
float last_valid_data_ch1[5];
float last_valid_data_ch2[5];
float last_valid_data_ch3[5];
float last_valid_data_ch4[5];

//for tuning frequency
int count = 0;
int max = 60;
int strain_coe = 700;

void send_data(float x[5],float y[5]){
    // creat socket
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    // socket address setting
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("192.168.177.1");
    serv_addr.sin_port = htons(9999);
    // connect socket
    connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    //send the processed data 
    std::ostringstream oss;
    for (int i = 0; i < 5; ++i)
    {
        oss << x[i] << " ";
    }
    for (int i = 0; i < 5; ++i)
    {
        oss << y[i] << " ";
    }
    std::string msg = oss.str();
    send(sock_fd, msg.c_str(), msg.length(), 0);
    close(sock_fd);

}

// i=13 wire no.7
// 0 3 6
// i=25 wire lastno. 8
void dataCallback(const std_msgs::Float32MultiArray::ConstPtr& msg)
{
    float wavelengths_ch1[5];
    float wavelengths_ch2[5];
    float wavelengths_ch3[5];
    float wavelengths_ch4[5];
    float Fx[5];
    float Fy[5];
    for(int i=0;i<=4;i++)
    {   
    //because the channel 4 no.13 sensor is broken, so we use no.12 for now
    int j = 0;
    if(i==4)j=1;
    wavelengths_ch1[i] = msg->data[3*i-j];
    wavelengths_ch2[i] = msg->data[13 + 3*i-j];
    wavelengths_ch3[i] = msg->data[26 + 3*i-j]; 
    wavelengths_ch4[i] = msg->data[39 + 3*i-j];
    }
    if(!flag)
    {   
        //set flag to true for skip the deflaut value setting
        flag = true;
        for (int i = 0; i < 5; i++) {
            offset_ch1[i] = wavelengths_ch1[i];
            offset_ch2[i] = wavelengths_ch2[i];
            offset_ch3[i] = wavelengths_ch3[i];
            offset_ch4[i] = wavelengths_ch4[i];
            last_valid_data_ch1[i] = wavelengths_ch1[i];
            last_valid_data_ch2[i] = wavelengths_ch2[i];
            last_valid_data_ch3[i] = wavelengths_ch3[i];
            last_valid_data_ch4[i] = wavelengths_ch4[i];

            //set the flag to false if initialization failed
            
            if(wavelengths_ch1[i]==-1 ||wavelengths_ch2[i]==-1 || wavelengths_ch3[i]==-1 ||wavelengths_ch4[i]==-1){
                flag = false;
                
                if(wavelengths_ch1[i]==-1 );
                    printf("strain_ch1:[%d],outside the windows,initialization failed\n",i);
                if(wavelengths_ch2[i]==-1 );
                    printf("strain_ch2:[%d],outside the windows,initialization failed\n",i);
                if(wavelengths_ch3[i]==-1 );
                    printf("strain_ch3:[%d],outside the windows,initialization failed\n",i);
                if(wavelengths_ch4[i]==-1 );
                    printf("strain_ch4:[%d],outside the windows,initialization failed\n",i);
                
            }
            
        }
        
    } else {
        if(count==max){   
            //calculate strain
            for (int i = 0; i < 5; i++) {
                if(wavelengths_ch1[i]==-1){
                    printf("wavelengths_ch1:[%d],outside the windows\n",i);
                }else{ 
                    last_valid_data_ch1[i] = wavelengths_ch1[i];  }
                if(wavelengths_ch2[i]==-1){
                    printf("wavelengths_ch2:[%d],outside the windows\n",i);
                }else{ 
                    last_valid_data_ch2[i] = wavelengths_ch2[i];  }
                if(wavelengths_ch3[i]==-1){
                    printf("wavelengths_ch3:[%d],outside the windows\n",i);
                }else{ 
                    last_valid_data_ch3[i] = wavelengths_ch3[i];  }
                if(wavelengths_ch4[i]==-1){
                    printf("wavelengths_ch4:[%d],outside the windows\n",i);
                }else{ 
                    last_valid_data_ch4[i] = wavelengths_ch4[i];  }    
                
                // channel1 only sensitive to strain and pressure, then we have the entire strain of the FBGs, 
                // then we can use it to compensate other FBGs at the same cross section 
                strain_ch1[i] = (last_valid_data_ch1[i] - offset_ch1[i])/offset_ch1[i];
                float offset_strain = last_valid_data_ch1[i] - offset_ch1[i];
                // we can calculate the shrink and elongate of ch2~4
                strain_ch2[i] = (last_valid_data_ch2[i] - offset_ch2[i] - offset_strain)/offset_ch2[i];
                strain_ch3[i] = (last_valid_data_ch3[i] - offset_ch3[i] - offset_strain)/offset_ch3[i];
                strain_ch4[i] = (last_valid_data_ch4[i] - offset_ch4[i] - offset_strain)/offset_ch4[i];
                printf("strain_ch1:[%d]%f\n",i,strain_ch1[i]); 
                printf("strain_ch2:[%d]%f\n",i,strain_ch2[i]);   
                printf("strain_ch3:[%d]%f\n",i,strain_ch3[i]);   
                printf("strain_ch4:[%d]%f\n",i,strain_ch4[i]);      

                double ang = 0;
                //calculate x and y
                double angle1 = M_PI/2 + ang;
                double angle2 = 7*M_PI/6 + ang;
                double angle3 = 11*M_PI/6 + ang;
                double F1x = strain_ch3[i]*cos(angle1);
                double F1y = strain_ch3[i]*sin(angle1);
                double F2x = strain_ch2[i]*cos(angle2);
                double F2y = strain_ch2[i]*sin(angle2);
                double F3x = strain_ch4[i]*cos(angle3);
                double F3y = strain_ch4[i]*sin(angle3);
                Fx[i] = (F1x + F2x + F3x)*strain_coe;
                Fy[i] = (F1y + F2y + F3y)*strain_coe;
            }
            send_data(Fx,Fy);
            count = 0;
        }
        else{
            count++;
        }
    }
    
}

int main(int argc, char *argv[])
{
    ros::init(argc, argv, "listener");
    ros::NodeHandle n;
    printf("start");
    ros::Subscriber sub = n.subscribe("wavelenghts",1000,dataCallback);
    ros::spin();
    return 0;
}

//for debug
/*
int main(int argc, char *argv[])
{   
    float base[5] = {1,1,1,1,1};
    float wavelengths[5] ;
    int changing_factor = 0;

    for(int j = 0; j<100;j++){

        if(j<20 || (j>40 && j< 60)){
            changing_factor++;}
        else{
            changing_factor--;}

        for(int wave = 0; wave<5; wave++){
            wavelengths[wave] = base[wave] + changing_factor * 0.04;}

        for (int i = 0; i < 5; i++) {
            strain[i] = (wavelengths[i] - base[i])/base[i];
            printf("strain:[%d]%f\n",i,strain[i]);}

        send_data(strain);
        sleep(0.5);
    }
    return 0;
}

*/