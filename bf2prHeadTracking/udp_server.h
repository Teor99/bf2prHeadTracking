#pragma once

struct CameraCoordsPacket {
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
	double yaw = 0.0;
	double pitch = 0.0;
	double roll = 0.0;
};

void udpServer();
void stopUdpServer();