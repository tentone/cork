#include <string>

/**
 * Stores the camera configuration to be used.
 */
class CameraConfiguration
{
public:
	const static int FILE = 0;
	const static int USB = 1;
	const static int IP = 2;
	const static int TCAM = 3;

	/**
	 * Indicates the camera input source.
	 */
	int inputSource;
	
	/**
	 * The USB camera number to use.
	 *
	 * Used when input source set to USB.
	 */
	int usbCamera;
	
	/**
	 * Serial number of the camera to use.
	 *
	 * Used when input source set to TCAM.
	 */
	std::string tcamSerial;

	/**
	 * Full IP address, username and password of the camera.
	 * 
	 * Used when input source set to IP.
	 *
	 * Examples rtsp://admin:123456@192.168.0.10:554/live/ch0 or rtsp://192.168.0.124:8080/video/h264
	 */
	std::string ipAddress;
};