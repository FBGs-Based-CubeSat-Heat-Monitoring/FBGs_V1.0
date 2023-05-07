# FBGs_V1.0
### This is a very initial version, our calculations in ROS are limited to the fibres, but in fact you can transfer the propagation part of the Blender part to C++ in ROS as well.  
  
Since the file is too large to push on git.    
Here is the google dive link:https://drive.google.com/drive/folders/1RDbxBo2eF2iyXJox-veGH0fYvCDK7ynd?usp=sharing     
There are two zip file inside, one for the ROS system, another for FAZT_I4.  
The way to use the ROS image is descripted in two part below(**Virtual machine and ROS system**).   
  
### **The order in which this document is read is not fixed, it simply acts like a user manual. You can read it once from start to finish, read it carefully if you are unsure, and come back to it if something goes wrong.**  
  
## **For FemtoSense 1.8**   
This software cannot be used directly in our projects, but it will give you an initial insight into our sensor data and it is highly recommended that you follow the installation process carefully as described in the instructions. After successful installation, you can use it to observe the wavelength variations of different channels under different deformations.   
Please note that this software should not be run in conjunction with TOPIC on the ROS system (no disaster, the system will tell you that the interface is already occupied).
  
## **Setting of IP and PORT**  
(if you don't know which ip you should use, check the 'Virtual machine and IP address' part, **the number of port is not important** as long as you set both side to same number remain between 1~9999 is fine)  
For Blender side  
 Line 377-378  
For ROS side  
 Line 38-39  
  
## **For connect to the interrogator**  
(Only for Windows, Linux can use terminal to do so.)   
From Settings of you system, choose 'change the adapter options' and right click your 'Ethemet net' to modify you TCP/IPv4 Properties, choose 'Use the following IP address' and enter '10.100.51.x'(x can be any number below 255 **except 0,22**)  
You can use FemtoSense 1.8 to check if you can connect to the interrogator or check how the wavelength looks like.  
  
## **For Virtual machine and IP address**   
Using a virtual machine tool such as VMware, select Open Virtual Machine and then select the ROS.ovf file in the file. Make sure that the hard disk size available for the virtual machine is more than 20GB. Use more than 2 cores and 4GB of memory to avoid crashing.  
  
For the IP settings, we recommend using VMnet8 as the IP of your virtual machine and connecting to the host via NAT so that you can keep your virtual machine connected to the Internet.  
  
Then you can check you ip address via you Windows or Linux  
**For windows cmd: input 'ipconfig' and search the VMnet8 IPv4 Address  
For Linux terminal: input 'ip a' and search the VMnet8 IPv4 Address**
    
    
## **For ROS system:**  
The **password** of the ROS system: **ros**  
The file 'ROS_image' including three files (ROS.mf, ROS.ovf, ROS-disk1.vmdk). Please make sure to save them completely in the same folder.   

Most of your algorithms should be carried out in this section.   
You can find the listener node code in **/home/ros/ros_rt_ws/src/ros_c/src/ros_c.cpp**, also you can open it via other IDEs.  
Currently you will see inside the code **line 145** there is a double varable call 'ang', set it to zero to get a good start.     
  
The code of the topic is saved in **/home/ros/wsfaz/src/rtt_fazt-master/src/rtt_fazt-component.cpp** , normally you only use it to set the **meanwave length and the windows size: Line41-44**.  
    
Currently we use VScodeIDE to modified and run our code, we set **Ctrl + Shift + B** compile the entire working space. So every time you modifiy the code, use **ctrl + shift + B** to compile.  
Feel free to change anything in confortable way.
   

    
    
## **For Blender:**  
**Open you Blender with powerShell when you want to receive the data from ROS system.**  
You can right click you Blender's logo to file location, inside the location use **'shift + rigth click'**, and you will see **'open powerShell window here'**, enter '**./blender**'.  
Click the 'Scripting' on the top bar and open a new draft, code of Blender can be paste. Or using 'open file' is fine.
What you print and recevied from ROS will be shown in the powerShell window but not blender.  
**If you just want you check the model with self made data, you don't need the powerShell.**
A small part of your initial algorithm may be carried out in this section.   
Setting of the Hyper parameter is in line 353-358.   
Here are some explanation and limitation in the hyper parameter setting part:   
1.radius and height is easy to understand,  
2.building_node is the number of the node in each layer, currently you can only set it to multiples of 4,   
3.offset_z is used to compensate for the heat shield lift caused by the creation of an inflatable Ring at (0,0,0),  
4.layer it means the number of layer between nosecone and the bottom layer, We have not looked for a more general algorithm to fit all cases, but you can still see how it is deconstructed in this part of the code (lines 37-46). We suggest that you change layer5 to 6 or 4 to see how it changes and to make some improvements.  
Also in the sensor_read2 function starting at line 248. We are trying to extend the propagation to the other 3 FBGs. This part of the code is not entirely correct and needs further debugging.Of course, if the data handling is implemented in C++, it is not necessary to take care of it.    

    

