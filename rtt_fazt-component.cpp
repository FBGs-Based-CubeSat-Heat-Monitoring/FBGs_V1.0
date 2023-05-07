#include "rtt_fazt-component.hpp"
#include <rtt/Component.hpp>
#include <iostream>

using namespace RTT ;
rtt_fazt::rtt_fazt(std::string const& name) : TaskContext(name,PreOperational)
  , prop_adress("10.100.51.33")
  , port_number(9931)
  , error_event("e_"+this->getName()+"_error")
  , check_window(false)
  , counter_max(5)
{


	addProperty("psrt_number",port_number);
	addProperty("interrogator_adress",prop_adress);
	addProperty("sub_sampling",counter_max).doc("Sub sampling for visualization purpouse");

    addProperty("sensorConfiguration",bag).doc("configuration of the sensors");
    bag.addProperty("check_window",check_window).doc("switch on or off  the check of window wavelenght");
    bag.addProperty("wavelenght_mean",wavelenght_mean).doc("mean values of the wavelenghts");
    bag.addProperty("window_size",window_size).doc("the signals are expected in mean+-window_size/2");

	addPort("period_outport",period_outport).doc("time elapsed between two triggers of the update hook");
	addPort("wavelenghts_outport",wavelenghts_outport).doc("wavelenghts[m], set  -2 if value is out of bounds (channel properties)");
	addPort("wavelenghts_vis_outport",wavelenghts_vis_outport).doc("for_visualization");
	addPort("event_outport",event_outport).doc("signal errors");

	act = new RTT::extras::FileDescriptorActivity(os::HighestPriority);
	this->setActivity(act);
	act = dynamic_cast<RTT::extras::FileDescriptorActivity*>(this->getActivity());


	//init with what I expect...
	n_of_channels=4;
	n_of_sensors=13;
	wavelenghts.resize(n_of_channels*n_of_sensors,-1.5);
	wavelenghts_vis.data.resize(n_of_channels*n_of_sensors,-1.5);
	wavelenghts_outport.setDataSample(wavelenghts);
	wavelenghts_vis_outport.setDataSample(wavelenghts_vis);
	window_size.resize(13,0.0030e-6);
	wavelenght_mean={1.530e-6,  1.533e-6 ,  1.536e-6 ,  1.539e-6  ,  1.542e-6 ,
					 1.545e-6  ,  1.548e-6   ,  1.551e-6 ,  1.554e-6,  1.557e-6,
					 1.560e-6 ,  1.563e-6,  1.566e-6 };

	counter=counter_max;
}	

bool rtt_fazt::configureHook(){

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		Logger::In in(this->getName());
		log(Error)<<this->getName()<<": not able to open the socket..." << endlog();
		return false;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port=htons(port_number);


	//Convert from presentation format to an Internet number
	if(inet_pton(AF_INET, prop_adress.c_str(), &serv_addr.sin_addr)<=0)
	{
		Logger::In in(this->getName());
		log(Error)<<this->getName()<<":the string "<<prop_adress
				 <<" is not a good formatted string for address ( like 127.0.0.1)"
				<< endlog();
		///add log from other file
		return false;	
	}
	return true;
}

bool rtt_fazt::startHook(){
	//check property sizes;
	if(window_size.size()!=n_of_sensors ||
	   wavelenght_mean.size()!=n_of_sensors )	{
		Logger::In in(this->getName());
		log(Error)<<this->getName()<<"window_size and/or wavelenght_mean have wrong size!"<< endlog();
		return false;
	}


	if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		Logger::In in(this->getName());
		log(Error)<<this->getName()<<": Connection failed!"<< endlog();
		return false;
	}
	log(Info)<<this->getName()<<": Connection OK!"<< endlog();
	//activity set watch
	act->watch(sockfd);
	act->setTimeout(2000);
	m_time_begin = os::TimeService::Instance()->getTicks();
	first=true;
	return true;
}

void rtt_fazt::updateHook(){
	m_time_passed = os::TimeService::Instance()->secondsSince(m_time_begin);
	m_time_begin = os::TimeService::Instance()->getTicks();
	period_outport.write(m_time_passed);

	if(act->hasError()){
		Logger::In in(this->getName());
		log(Error)  <<this->getName()<<" socket error - unwatching all sockets. restart the component" << endlog();
		act->clearAllWatches();
		close(sockfd);
		this->stop();
		this->cleanup();
	}
	else if (act->hasTimeout()){
		Logger::In in(this->getName());
		log(Error)  <<this->getName()<<" socket timeout" << endlog();

	}
	else{
		if(act->isUpdated(sockfd)){

			int data_to_read=0;
			int data_read=0;
			int n;
			data_to_read=16;

			//read one
			read_exactly(sockfd,data_to_read,header_buffer);
			deserializeHeader(header_buffer,header);

			//read the rest of the package
			data_to_read=header.data_lenght+header.data_offset-8;

			//read two
			read_exactly(sockfd,data_to_read,buffer);
			n=deserializeData(buffer,header,error,peaks,tail);
			if(n<0){
				Logger::In in(this->getName());
				log(Error)  <<this->getName()<<" Error in deserilising data" << endlog();
				event_outport.write(error_event);
				this->stop();
			}
			/*


			n=deserializeErrorPayload(buffer,header,error);
			if(n<0){
				Logger::In in(this->getName());
				log(Error)  <<this->getName()<<" Error in deserilising data (Error Part)" << endlog();
				event_outport.write(error_event);
				this->stop();
			}
			if(n!=0){
				std::cout<<"ERRORs  expected: "<<nErrorsPresent(header)<<std::endl;
				std::cout<<"n: \t"<<n<<std::endl;
				std::cout<<"do: \t"<<header.data_offset<<std::endl;
				data_read+=n;
			}



			n=deserializePeakPayload(buffer+data_read,header,peaks);
			if(n<0){
				Logger::In in(this->getName());
				log(Error)  <<this->getName()<<" Error in deserilising data (Peak Part)" << endlog();
				event_outport.write(error_event);
				this->stop();

			}
			data_read+=n;

			n=deserializeTail(buffer+data_read,tail);
			if(n<0){
				Logger::In in(this->getName());
				log(Error)  <<this->getName()<<" Error in deserilising data (Tail Part)" << endlog();
				event_outport.write(error_event);
				this->stop();
			}
			*/
			for (int i_ch=0;i_ch<n_of_channels;i_ch++)
				for (int i_sen=0;i_sen<n_of_sensors;i_sen++)
				{
					bool ok=false;
					for(uint i=0;i<peaks.size();i++)
					{
						if(i_ch==peaks[i].peak_ID.channel_ID  &&
						   i_sen==peaks[i].peak_ID.sensor_ID){
							wavelenghts[i_ch*n_of_sensors+i_sen]=peaks[i].peak_wavelenght;
							ok=true;
							break;
						}
					}
					if(!ok) {
						wavelenghts[i_ch*n_of_sensors+i_sen]=-1;
						Logger::In in(this->getName());
						log(Warning) <<": peak data not found \t "<<i_ch<<"\t"<<i_sen << endlog();
					}
					if(check_window){
						double wavelenght_min=(wavelenght_mean[i_sen]-window_size[i_sen]/2);
						double wavelenght_max=(wavelenght_mean[i_sen]+window_size[i_sen]/2);
						if(wavelenghts[i_ch*n_of_sensors+i_sen]<wavelenght_min ||
						   wavelenghts[i_ch*n_of_sensors+i_sen]>wavelenght_max){
							/*	std::cout<<"out of bound\t "<<i_ch<<"\t"<<i_sen<<"\t"<<
								   wavelenghts[i_ch*n_of_sensors+i_sen]<<"\t"<<
								   wavelenght_min<<"\t"<<
								  wavelenght_max;
						if(wavelenghts[i_ch*n_of_sensors+i_sen]<wavelenght_min)
							std::cout<<"\tsmaller\t"<< wavelenghts[i_ch*n_of_sensors+i_sen]-wavelenght_min <<std::endl;
						else std::cout<<"\tbigger\t"<< wavelenghts[i_ch*n_of_sensors+i_sen]-wavelenght_max<<std::endl;
					*/
							wavelenghts[i_ch*n_of_sensors+i_sen]=-2;
						}
					}
				}
			wavelenghts_outport.write(wavelenghts);
			if (counter==0)
			{
			wavelenghts_vis.data.resize(wavelenghts.size());
			for (uint i=0;i<wavelenghts.size();i++)
				wavelenghts_vis.data[i]=wavelenghts[i];
			wavelenghts_vis_outport.write(wavelenghts_vis);
			counter=counter_max;
			}else counter --;






		}
	}
}

void rtt_fazt::stopHook() {
	act->clearAllWatches();
	close(sockfd);
}

void rtt_fazt::cleanupHook() {
	std::cout << "rtt_fazt cleaning up !" <<std::endl;
}

/*
 * Using this macro, only one component may live
 * in one library *and* you may *not* link this library
 * with another component library. Use
 * ORO_CREATE_COMPONENT_TYPE()
 * ORO_LIST_COMPONENT_TYPE(rtt_fazt)
 * In case you want to link with another library that
 * already contains components.
 *
 * If you have put your component class
 * in a namespace, don't forget to add it here too:
 */
ORO_CREATE_COMPONENT(rtt_fazt)