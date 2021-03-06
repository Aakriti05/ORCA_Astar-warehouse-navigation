/*
 * Blocks.cpp
 * RVO2 Library
 *
 * Copyright 2008 University of North Carolina at Chapel Hill
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please send all bug reports to <geom@cs.unc.edu>.
 *
 * The authors may be contacted via:
 *
 * Jur van den Berg, Stephen J. Guy, Jamie Snape, Ming C. Lin, Dinesh Manocha
 * Dept. of Computer Science
 * 201 S. Columbia St.
 * Frederick P. Brooks, Jr. Computer Science Bldg.
 * Chapel Hill, N.C. 27599-3175
 * United States of America
 *
 * <http://gamma.cs.unc.edu/RVO2/>
 */

/*
 * Example file showing a demo with 100 agents split in four groups initially
 * positioned in four corners of the environment. Each agent attempts to move to
 * other side of the environment through a narrow passage generated by four
 * obstacles. There is no roadmap to guide the agents around the obstacles.
 */

#ifndef RVO_OUTPUT_TIME_AND_POSITIONS
#define RVO_OUTPUT_TIME_AND_POSITIONS 1
#endif

#ifndef RVO_SEED_RANDOM_NUMBER_GENERATOR
#define RVO_SEED_RANDOM_NUMBER_GENERATOR 1
#endif

#include <cmath>
#include <cstdlib>

#include <vector>

#if RVO_OUTPUT_TIME_AND_POSITIONS
#include <iostream>
#include <fstream>
#endif

#if RVO_SEED_RANDOM_NUMBER_GENERATOR
#include <ctime>
#endif

#if _OPENMP
#include <omp.h>
#endif

#include <RVO.h>
#include <stdio.h>
#include "sbpl_includes.h"
# include <graphics.h>
// #include <conio.h>
// #include <alloc.h>
// # include <dos.h>



#include <boost/thread.hpp>
//Socket related includes
#include <unistd.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <arpa/inet.h>



#ifndef M_PI
const float M_PI = 3.14159265358979323846f;
#endif

/* Store the goals of the agents. */

std::vector<RVO::Vector2> global_goals;

std::vector<std::vector<RVO::Vector2> > tasks_all;
std::vector<RVO::Vector2> tasks_onerobot;

/**Variable to enable/disable TCP comm thread*/
bool comm_thread_running;
int new_socket;
int server_fd;
struct sockaddr_in address;
int opt; 
int addrlen; 
bool listen_thread_running;
boost::thread *listen_thread;
/**Boost thread for sending data to robot*/
boost::thread *comm_thread_send;
/**Boost thread for receiving data from robot*/
boost::thread *comm_thread_recv;


struct robot
{
	double robot_x;
    double robot_y;
	int comm_soc;
	int id;
	bool global_reached;
	bool local_reached;
	std::vector<double> start;
	std::vector<double> end;
	RVO::Vector2 current_pos;

	std::vector<std::vector<double> > current_full_path;
	RVO::Vector2 local_goals;	
	std::vector<double> next_waypoint;
	std::vector<std::vector<int> > full_solution;

};  
const int robot_no = 20;
struct robot robots[robot_no];
int num_robots_connected;
double robot_pos_x;
double robot_pos_y;

int collision = 0;
void *arrow;



#define CELL_GRID_SIZE 0.5


// global planning 

std::string searchDir = "forward";
std::string plannerType = "arastar";
PlannerType planner = StrToPlannerType(plannerType.c_str());
char * env_file_name = (char *)"../env_sorter.cfg";
char * mot_prim_file_name = (char *)"../test.mprim";
bool forwardSearch = !strcmp(searchDir.c_str(), "forward");	

unsigned char **grid ;
int grid_height = 100;
int grid_width = 100;
// //bool first_run = false;
int dx = -100;
int dy = -100;
bool request_replan = false;
int **reservation_table;




#define PORT 8080


void draw_background(){
	std::string line;
	ifstream myfile ("../env_sorter.cfg");

	int row = 0;
	int col = 0;
	if (myfile.is_open())
	{
		while ( getline (myfile,line) )
		{
			row = row+1;
			col = 0;
			if(row > 10) {
				for(col = 0; col<122 ; col++){
					if(line[2*col] == '0') putpixel(col+50, 88-(row-10)+50, BLACK);
					else if (line[2*col] == '1') putpixel(col+50, 88-(row-10)+50, WHITE);
				}
			}
		}
		// std::cout << "row: " << row << endl;
		myfile.close();
	}
}



void setupScenario(RVO::RVOSimulator *sim)
{
#if RVO_SEED_RANDOM_NUMBER_GENERATOR
	std::srand(static_cast<unsigned int>(std::time(NULL)));
#endif

	/* Specify the global time step of the simulation. */
	sim->setTimeStep(0.25f);

	/* Specify the default parameters for agents that are subsequently added. */
	sim->setAgentDefaults(15.0f, 10, 5.0f, 5.0f, 0.25f, 2.0f);

	/*
	 * Add agents, specifying their start position, and store their goals on the
	 * opposite side of the environment.
	 */

	for (size_t i = 0; i < robot_no; ++i) {

		if(i==0) sim->addAgent(RVO::Vector2(40, 8)); //sim->addAgent(RVO::Vector2(5*(-0.65+ 0.6*i +6), 5*(-3.95 + 4.3))); F
		else if(i==1) sim->addAgent(RVO::Vector2(96, 78)); //B 48.25, 39.25
		else if(i==2) sim->addAgent(RVO::Vector2(38, 8)); //F 19.25, 4
		else if(i==3) sim->addAgent(RVO::Vector2(71, 3)); //71, 3 A
		else if(i==4) sim->addAgent(RVO::Vector2(98, 4)); //44, 2.25 C
		else if(i==5) sim->addAgent(RVO::Vector2(100, 4)); //50.25, 4 C
		else if(i==6) sim->addAgent(RVO::Vector2(23, 78)); // 11.5, 78 D
		else if(i==7) sim->addAgent(RVO::Vector2(19, 78)); // D = 9.5, 78
		else if(i==8) sim->addAgent(RVO::Vector2(98, 78)); // B = 98, 78
		else if(i==9) sim->addAgent(RVO::Vector2(65, 5)); // A 32.75, 2.75

		else if(i==10) sim->addAgent(RVO::Vector2(102, 4)); //C 51.25, 4
		else if(i==11) sim->addAgent(RVO::Vector2(104, 4)); //C 54, 4
		else if(i==12) sim->addAgent(RVO::Vector2(100, 78)); //B = 51.25, 78
		else if(i==13) sim->addAgent(RVO::Vector2(106, 4)); //C 53.25, 2.25
		else if(i==14) sim->addAgent(RVO::Vector2(25, 78)); //12.5, 78 D
		else if(i==15) sim->addAgent(RVO::Vector2(108, 4)); //54.25, 2.25 C
		else if(i==16) sim->addAgent(RVO::Vector2(65, 7)); // 32.75, 3.75 A
		else if(i==17) sim->addAgent(RVO::Vector2(102, 78)); // B = 51.25, 78
		else if(i==18) sim->addAgent(RVO::Vector2(94, 78)); // B = 47.25, 78
		else if(i==19) sim->addAgent(RVO::Vector2(36, 8)); // F 18.25, 4
		robots[i].start.clear();
		robots[i].start.resize(0);
		robots[i].end.clear();
		robots[i].end.resize(0);

		// std::cout<< i << " agent added " << endl;
	
		if(i==0){
			robots[i].start.push_back(20.25); 			robots[i].start.push_back(4.25);		} 
		else if(i==1){
			robots[i].start.push_back(48.25);			robots[i].start.push_back(39.25);		}
		else if(i==2){
			robots[i].start.push_back(19.25);			robots[i].start.push_back(4.25);		}
		else if(i==3){
			robots[i].start.push_back(35.75);			robots[i].start.push_back(1.75);		}
		else if(i==4){
			robots[i].start.push_back(49.25);			robots[i].start.push_back(2.25);		}
		else if(i==5){
			robots[i].start.push_back(50.25);			robots[i].start.push_back(2.25);		}
		else if(i==6){
			robots[i].start.push_back(11.5);			robots[i].start.push_back(39.25);		}
		else if(i==7){
			robots[i].start.push_back(9.5);		    	robots[i].start.push_back(39.25);		}
		else if(i==8){
			robots[i].start.push_back(48.25);			robots[i].start.push_back(39.25);		}
		else if(i==9){
			robots[i].start.push_back(32.75);			robots[i].start.push_back(2.75);		}
		else if(i==10){
			robots[i].start.push_back(51.25);			robots[i].start.push_back(2.25);		} 
		else if(i==11){
			robots[i].start.push_back(52.25);			robots[i].start.push_back(2.25);		}
		else if(i==12){
			robots[i].start.push_back(50.25);			robots[i].start.push_back(39.25);		}
		else if(i==13){
			robots[i].start.push_back(53.25);			robots[i].start.push_back(2.25);		}
		else if(i==14){
			robots[i].start.push_back(12.5);			robots[i].start.push_back(39.25);		}
		else if(i==15){
			robots[i].start.push_back(54.25);			robots[i].start.push_back(2.25);		}
		else if(i==16){
			robots[i].start.push_back(32.75);			robots[i].start.push_back(3.75);		}
		else if(i==17){
			robots[i].start.push_back(51.25);			robots[i].start.push_back(39.25);		}
		else if(i==18){
			robots[i].start.push_back(47.25);			robots[i].start.push_back(39.25);		}
		else if(i==19){
			robots[i].start.push_back(18.25);			robots[i].start.push_back(4.25);		}

		std::cout<< i << " start added" << endl;
		robots[i].start.push_back(0);

		robots[i].end.push_back(global_goals[i].x());
		robots[i].end.push_back(global_goals[i].y());
		robots[i].end.push_back(0);

		std::cout<< i << " goal added" << endl;
		std::vector<std::vector<double> > to_;
		to_.push_back(robots[i].end);
		robots[i].full_solution = planxythetalat(planner, env_file_name, mot_prim_file_name, forwardSearch, robots[i].start, to_, &grid, &grid_height, &grid_width, false, request_replan, dx, dy, reservation_table, 1);

		for(int j = 0; j < robots[i].current_full_path.size(); j++)
		{
			robots[i].current_full_path[j].clear();
			robots[i].current_full_path[j].resize(0);
		}
		robots[i].current_full_path.clear();
		robots[i].current_full_path.resize(0);


		for(int j = 0; j < robots[i].full_solution.size(); j++)
		{
			if (robots[i].full_solution[j].size() == 3){
				std::vector<double> v;
				v.push_back(float(robots[i].full_solution[j][0])); // v.push_back((float(robots[i].full_solution[j][0])/10) - 6);
				v.push_back(float(robots[i].full_solution[j][1]));// v.push_back((float(robots[i].full_solution[j][1])/10) - 4.3);
				v.push_back(DiscTheta2Cont(robots[i].full_solution[j][2], 4));

				if(v.size() == 3){
					robots[i].current_full_path.push_back(v);
					std::cout<< "current full path " << v[0] << " " << v[1] << endl;
				}
			}
		}
	}
}

#if RVO_OUTPUT_TIME_AND_POSITIONS
void updateVisualization(RVO::RVOSimulator *sim)
{
	/* Output the current global time. */
	// std::cout << sim->getGlobalTime();

	// draw_background();

	/* Output the current position of all the agents. */
	for (size_t i = 0; i < sim->getNumAgents(); ++i) {
		// std::cout << " " << sim->getAgentPosition(i);
		RVO::Vector2 posit = sim->getAgentPosition(i);
		// if(i==0) std::cout << "position: " << int(posit.x()) << " " << int(posit.y());
		circle((int(posit.x())+50), (int(posit.y())+50), 0.5);
		setcolor((i+1)%15);
	}

	for (size_t i = 0; i < sim->getNumAgents(); ++i) {
		// std::cout << " " << sim->getAgentPosition(i);
		RVO::Vector2 posit = sim->getAgentPosition(i);
		// if(i==0) std::cout << "position: " << int(posit.x()) << " " << int(posit.y());
		putpixel((int(posit.x())+50), (int(posit.y())+50), BLACK);
		// circle((int(posit.x())+50), (int(posit.y())+50), 0.5);
		// setcolor((i+1)%15);
	}

	// delay(10);

	// std::cout << " " << std::endl;
}
#endif

void setPreferredVelocities(RVO::RVOSimulator *sim)
{
	/*
	 * Set the preferred velocity to be a vector of unit magnitude (speed) in the
	 * direction of the goal.
	 */
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int i = 0; i < static_cast<int>(sim->getNumAgents()); ++i) {
		RVO::Vector2 goalVector = robots[i].local_goals - sim->getAgentPosition(i);

		if (RVO::absSq(goalVector) > 1.0f) {
			goalVector = RVO::normalize(goalVector);
		}

		sim->setAgentPrefVelocity(i, goalVector);		


		// std::cout<< "pref velo: " << 	goalVector << endl;
		/*
		 * Perturb a little to avoid deadlocks due to perfect symmetry.
		 */
		float angle = std::rand() * 2.0f * M_PI / RAND_MAX;
		float dist = std::rand() * 0.0001f / RAND_MAX;

		sim->setAgentPrefVelocity(i, sim->getAgentPrefVelocity(i) +
		                          dist * RVO::Vector2(std::cos(angle), std::sin(angle)));

		if(robots[i].global_reached == true && tasks_all[i].size() == 0){
			sim->setAgentPrefVelocity(i, RVO::Vector2(0,0));
		}

		//calculating collision number

		for(int j=0; j < static_cast<int>(sim->getNumAgents()); ++j){
			if (j==i) continue;
			float x_sq = pow(sim->getAgentPosition(i).x() - sim->getAgentPosition(j).x(), 2);
			float y_sq = pow(sim->getAgentPosition(i).y() - sim->getAgentPosition(j).y(), 2);
			if (pow(x_sq + y_sq, 0.5) < 0.5f){
				collision = collision+1;
				std::cout << "collision: " << collision << endl;
			}
		}
	}
}

bool reachedGoal(RVO::RVOSimulator *sim)
{
	/* Check if all agents have reached their goals. */
	// printf("No of agents: ", sim->getNumAgents());
	for (size_t i = 0; i < sim->getNumAgents(); ++i) {
		if (RVO::absSq(sim->getAgentPosition(i) - global_goals[i]) > 20.0f * 20.0f) {
			return false;
		}
	}

	return true;
}

void communicate_recv()
{
	while(comm_thread_running)
	{
		// printf("enter recieve");
		char recvbuffer[1024] = {0};
		int valread = 0;
		valread = read(new_socket, recvbuffer, 1024);
		if(valread > 0)
		{
			sscanf(recvbuffer, "A,%lf,%lf,B", &robot_pos_x, &robot_pos_y);
		}
		usleep(10000);
	} 
}

void communicate_send()
{
	// printf("Robot %d started communication\n", id);
	while(comm_thread_running)
	{
		// printf("enter send");
		int val_send = 0;
		char sendbuffer[1024] = {0};
		sprintf(sendbuffer, "A,%0.3lf,%0.3lf,%0.3lf,B", 1.1, 1.1, 0.45);
		val_send = send(new_socket, sendbuffer, strlen(sendbuffer), 0);
		usleep(10000);
	} 
}

void start_communication()
{
	comm_thread_running = true;
	comm_thread_send = new boost::thread(boost::bind(&communicate_send));
	comm_thread_recv = new boost::thread(boost::bind(&communicate_recv));
}

void listen_for_connections()
{
	while(listen_thread_running)
	{
		if(listen(server_fd, 100) < 0)
		{
			printf("Listening for connection\n");
			//exit(EXIT_FAILURE); 
		}
		else
		{
			if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
							(socklen_t*)&addrlen))<0)
			{
				perror("accept");
				//exit(EXIT_FAILURE); 
			}
			else
			{
				printf("IP Address ::%s\n",inet_ntoa(address.sin_addr));

				robots[num_robots_connected].comm_soc = new_socket;
				robots[num_robots_connected].id = num_robots_connected;
				start_communication(); // robots[num_robots_connected].start_communication();
				num_robots_connected++;
				printf("no_of_robots: %d and %d\n", num_robots_connected, new_socket);
			}
		}
		usleep(100000);
	}
}

void start_server()
{
	opt = 1; 
	addrlen = sizeof(address); 

	if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 

	// Forcefully attaching socket to the port 8080 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 

	// Forcefully attaching socket to the port 8080 
	if (bind(server_fd, (struct sockaddr *)&address,  sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	printf("Server binding successful. Listening for incoming connections on port %d\n", PORT);
	listen_thread_running = true;
	listen_thread = new boost::thread(boost::bind(&listen_for_connections));
}

void task_allocate_to_eachrobot()
{
	// defining tasks 
	//robot 1
	tasks_onerobot.push_back(RVO::Vector2(9.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(26.75, 1.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(49.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(26.75, 1.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(49.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(9.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot2
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(29.75, 1.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(48.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(10.5, 39.25)); //D 
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(29.75, 1.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(48.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(29.75, 1.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(48.25, 39.25)); //B
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot3
	tasks_onerobot.push_back(RVO::Vector2(11.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(32.75, 1.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(50.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(32.75, 1.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(50.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(32.75, 1.75)); //A
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot4
	tasks_onerobot.push_back(RVO::Vector2(51.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(12.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(20.25, 4.25)); //F
	tasks_onerobot.push_back(RVO::Vector2(12.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(35.75, 1.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(51.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot5
	tasks_onerobot.push_back(RVO::Vector2(38.75, 1.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(50.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(38.75, 1.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(50.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(9.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(38.75, 1.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(50.25, 39.25)); //B
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot6
	tasks_onerobot.push_back(RVO::Vector2(41.75, 1.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(49.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(10.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(20.25, 4.25)); //F
	tasks_onerobot.push_back(RVO::Vector2(10.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(49.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(50.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(41.75, 1.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(49.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(10.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot7
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(20.25, 4.25)); //F
	tasks_onerobot.push_back(RVO::Vector2(11.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(20.25, 4.25)); //F
	tasks_onerobot.push_back(RVO::Vector2(11.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(48.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(44.75, 1.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(48.25, 39.25)); //B
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot 8 D = 9.5, 39.25
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(26.75, 2.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(49.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(9.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(20.25, 4.25)); //F
	tasks_onerobot.push_back(RVO::Vector2(9.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E	
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot9 B = 48.25, 39.25
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(29.75, 2.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(49.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(29.75, 2.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(49.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(29.75, 2.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(49.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(29.75, 2.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(49.25, 39.25)); //B
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot10 A 32.75, 2.75
	tasks_onerobot.push_back(RVO::Vector2(50.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(11.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(20.25, 4.25)); //F
	tasks_onerobot.push_back(RVO::Vector2(11.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(32.75, 2.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(50.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(11.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot 11
	tasks_onerobot.push_back(RVO::Vector2(35.75, 2.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(50.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(9.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(51.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(35.75, 2.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(50.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(51.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(35.75, 2.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(50.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(51.25, 2.25)); //C
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot12
	tasks_onerobot.push_back(RVO::Vector2(38.75, 2.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(51.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(52.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(38.75, 2.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(51.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(52.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(38.75, 2.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(51.25, 39.25)); //B	
	tasks_onerobot.push_back(RVO::Vector2(10.5, 39.25)); //D 
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot13
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(41.75, 2.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(51.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(10.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(20.25, 4.25)); //F
	tasks_onerobot.push_back(RVO::Vector2(10.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(20.25, 4.25)); //F
	tasks_onerobot.push_back(RVO::Vector2(10.5, 39.25)); //D
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot14
	tasks_onerobot.push_back(RVO::Vector2(44.75, 2.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(51.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(53.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(44.75, 2.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(51.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(10.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(53.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(44.75, 2.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(51.25, 39.25)); //B
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot15
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(26.75, 3.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(48.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(26.75, 3.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(48.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(12.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(20.25, 4.25)); //F
	tasks_onerobot.push_back(RVO::Vector2(12.5, 39.25)); //D
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot16
	tasks_onerobot.push_back(RVO::Vector2(29.75, 3.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(48.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(11.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(20.25, 4.25)); //F
	tasks_onerobot.push_back(RVO::Vector2(11.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(29.75, 3.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(48.25, 39.25)); //B
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot17
	tasks_onerobot.push_back(RVO::Vector2(48.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(11.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(20.25, 4.25)); //F
	tasks_onerobot.push_back(RVO::Vector2(11.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(48.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(32.75, 3.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(48.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(11.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot 18
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(35.75, 3.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(49.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(11.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(35.75, 3.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(49.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(35.75, 3.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(49.25, 39.25)); //B
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot19
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(38.75, 3.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(48.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(38.75, 3.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(48.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(12.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(20.25, 4.25)); //F
	tasks_onerobot.push_back(RVO::Vector2(12.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);

	//robot20
	tasks_onerobot.push_back(RVO::Vector2(12.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(20.25, 4.25)); //F
	tasks_onerobot.push_back(RVO::Vector2(12.5, 39.25)); //D
	tasks_onerobot.push_back(RVO::Vector2(5.25, 2.25)); //E
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_onerobot.push_back(RVO::Vector2(41.75, 3.75)); //A
	tasks_onerobot.push_back(RVO::Vector2(50.25, 39.25)); //B
	tasks_onerobot.push_back(RVO::Vector2(49.25, 2.25)); //C
	tasks_all.push_back(tasks_onerobot);
	tasks_onerobot.clear();
	tasks_onerobot.resize(0);


	for (int i=0; i< robot_no; i++){
		global_goals.push_back(tasks_all[i][0]);
		tasks_all[i].erase(tasks_all[i].begin());
	}
}

bool check_while(){
	for(int i=0; i<20; i++){
		if((tasks_all[i].size() == 0)  && (robots[i].global_reached == false)) return true;
		if(tasks_all[i].size() > 0) return true;
	}
	return false;
}

bool check_global_reach(){
	for(int i=0; i<20; i++){
		if(robots[i].global_reached == true) return true;
	}
	return false;
}


int main()
{
	/* Create a new simulator instance. */
	RVO::RVOSimulator *sim = new RVO::RVOSimulator();
	new_socket = -1;
	num_robots_connected = 0; 

	int gd=DETECT,gm;
	initgraph(&gd,&gm,"");	

	unsigned int size;

	// size = imagesize(50, 50, 172, 138);
	// arrow = malloc(size);

	draw_background();
	// int c = getpixel(50, 50);
	// getimage(50, 50, 172, 138, *arrow);
	// putimage(50, 50, arrow, 0);

	// start_server();
	task_allocate_to_eachrobot();
	/* Set up the scenario. */
	setupScenario(sim);

	for(int i=0; i<robot_no; i++){
		robots[i].local_reached = true;
		robots[i].global_reached = false;
		robots[i].next_waypoint = robots[i].current_full_path[0];
		robots[i].local_goals = RVO::Vector2(robots[i].next_waypoint[0], robots[i].next_waypoint[1]);
	}

	// setactivepage(1);

	while(check_while()){
		while(1){
			for (int i=0;i<robot_no;i++){
				robots[i].current_pos = sim->getAgentPosition(i);

				if(robots[i].local_reached){   
					if((fabs(robots[i].current_pos.x() - robots[i].current_full_path[0][0]) > 0.5) || (fabs(robots[i].current_pos.y() - robots[i].current_full_path[0][1]) > 0.5)){
						robots[i].next_waypoint = robots[i].current_full_path[0];
						robots[i].local_goals = RVO::Vector2(robots[i].next_waypoint[0], robots[i].next_waypoint[1]);
						robots[i].local_reached = false;
					}
					else{
						if(robots[i].current_full_path.size()>0){
							robots[i].current_full_path.erase(robots[i].current_full_path.begin());
						}	
					}	
				} 

				if((fabs(robots[i].current_pos.x() - robots[i].current_full_path[0][0]) < 0.5) && (fabs(robots[i].current_pos.y() - robots[i].current_full_path[0][1]) < 0.5)){
					robots[i].local_reached = true;		
				}

				if(robots[i].local_reached == true && robots[i].current_full_path.size() == 0){
					robots[i].global_reached = true;	
				}
			}

			#if RVO_OUTPUT_TIME_AND_POSITIONS
			updateVisualization(sim);
			#endif
			setPreferredVelocities(sim);
			// cleardevice();
			sim->doStep();

			if(check_global_reach()) break;
		}

		for (int i=0; i<robot_no; i++){
			if(robots[i].global_reached == true && (tasks_all[i].size() > 0)){
				global_goals[i] = tasks_all[i][0];
				tasks_all[i].erase(tasks_all[i].begin());
				//global planning

				robots[i].start.clear();
				robots[i].start.resize(0);
				robots[i].end.clear();
				robots[i].end.resize(0);
			
				robots[i].start.push_back(sim->getAgentPosition(i).x()/2);
				robots[i].start.push_back(sim->getAgentPosition(i).y()/2);
				robots[i].start.push_back(0);

				robots[i].end.push_back(global_goals[i].x());
				robots[i].end.push_back(global_goals[i].y());
				robots[i].end.push_back(0);

				std::cout<< "start: " << sim->getAgentPosition(i).x() << " " << sim->getAgentPosition(i).y() << endl;
				std::cout<< "end: " << global_goals[i].x() << " " << global_goals[i].y() << endl;

				std::vector<std::vector<double> > to_;
				to_.push_back(robots[i].end);
				robots[i].full_solution = planxythetalat(planner, env_file_name, mot_prim_file_name, forwardSearch, robots[i].start, to_, &grid, &grid_height, &grid_width, false, request_replan, dx, dy, reservation_table, 1);

				for(int j = 0; j < robots[i].current_full_path.size(); j++)
				{
					robots[i].current_full_path[j].clear();
					robots[i].current_full_path[j].resize(0);
				}
				robots[i].current_full_path.clear();
				robots[i].current_full_path.resize(0);


				for(int j = 0; j < robots[i].full_solution.size(); j++)
				{
					if (robots[i].full_solution[j].size() == 3){
						std::vector<double> v;
						v.push_back(float(robots[i].full_solution[j][0]));
						v.push_back(float(robots[i].full_solution[j][1]));
						v.push_back(DiscTheta2Cont(robots[i].full_solution[j][2], 4));

						if(v.size() == 3){
							robots[i].current_full_path.push_back(v);
						}
					}
				}
				robots[i].global_reached = false;
			}
		}		
	}		

	std::cout << "global time: " << sim->getGlobalTime();
	std::cout << "coll: " << collision;	

	if(comm_thread_running)
	{
		comm_thread_running = false;
		comm_thread_send->join();
		comm_thread_recv->join();
	}

	delete sim;
	closegraph();

	return 0;
}
