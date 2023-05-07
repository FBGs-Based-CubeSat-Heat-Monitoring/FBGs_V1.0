# FBGs_V1.0

**Setting of ip and port**(if you don't know which ip you should use, check the 'ip address' part, the number of port is not important,set both side same and between 1~9999 is fine)  
For Blender side  
 Line 377-378  
For ROS side  
 Line 38-39  
 
**For connect to the interrogator**(Only for Windows, Linux can use terminal to do so.)   
From Settings of you system, choose 'change the adapter options' and right click your 'Ethemet net' to modify you TCP/IPv4 Properties, choose 'Use the following IP address' and enter '10.100.51.x'(x can be any number below 255 **except 0,22**)  
  
**For ip address**   
Using a virtual machine tool such as VMware, select Open Virtual Machine and then select the ROS.ovf file in the file. Make sure that the hard disk size available for the virtual machine is more than 20GB. Use more than 2 cores and 4GB of memory to avoid crashing.  
  
For the IP settings, we recommend using VMnet8 as the IP of your virtual machine and connecting to the host via NAT so that you can keep your virtual machine connected to the Internet.  
  
Then you can check you ip address via you Windows or Linux  
**For windows cmd: input 'ipconfig' and search the VMnet8 IPv4 Address  
For Linux terminal: input 'ip a' and search the VMnet8 IPv4 Address**
    
**For Blender:**  
click the 'Scripting' on the top bar and open a new draft, code of Blender can be paste. Or using 'open file' is fine.  
  
  
    
**For ROS system:**  
The file 'ROS_image' including three files (ROS.mf, ROS.ovf, ROS-disk1.vmdk). Please make sure to save them completely in the same folder.   

  
Currently we use VScodeIDE to modified and run our code, we set **Ctrl + Shift + B** compile the entire working space.  
feel free to change it in confortable way.


