#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <time.h>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "src/Screen.hpp"
#include "src/agm.hpp"
#include "src/Camera.hpp"
#include "src/Lights.hpp"
#include "src/Mesh.hpp"

#include "yyjson.h"
#include "yyjson.c"

int gScreenWidth = 80; // Define the screens width and height
int gScreenHeight = 30;
float gAspect = (float)gScreenWidth / (float)gScreenHeight;

int main(int argc, char *argv[]) {
	FILE *sensor_file = NULL;
	char sensor_json[1028*10] = { 0 };
	char sensor_buffer[1028*10] = { 0 };
	char sensor_command[1028] = { 0 };
	const char *sensor_name = argv[1];
	char *checkpoint = NULL;
	time_t last = 0;
	time_t now;
	int c;
	if(argc < 2)
		sensor_name = "GeoMag Ro";

	/*char sensor_command[1028] = {
		"/data/data/com.termux/files/usr/bin/termux-sensor"
		" -d 500 -s GeoMag Ro"
	};*/

	/*char sensor_command[1028] = {
		"/data/data/com.termux/files/usr/bin/termux-sensor"
		" -d 500 -s GeoMag Ro"
	};*/

	snprintf(sensor_command, 1028,
		"/data/data/com.termux/files/usr/bin/termux-sensor"
		" -d 120 -s %s", sensor_name
	);

	sensor_file = popen(sensor_command, "r");
	//int flags = fcntl(fileno(sensor_file), F_GETFL);
	//flags &= O_NONBLOCK;
	fcntl(fileno(sensor_file), F_SETFL, O_NONBLOCK);

	while((c = fgetc(sensor_file)) == EOF){
		time(&now);
		if(last == 0){
			last = now;

			continue ;
		}

		if(!(now % 2)){
			printf("\b+");
		} else {
			printf("\bx");
		}

		if((now - last) < 3)
			continue ;

		last = now;
		printf("Sensor unresponsive. Retrying...\n ");
		/*pclose(sensor_file);
		sensor_file = popen(sensor_command, "r");
		fcntl(fileno(sensor_file), F_SETFL, O_NONBLOCK);
		printf("%i\n", c);*/
		exit(1);
	}

	ungetc(c, sensor_file);

	Camera camera(0.0f, 0.0f, 0.0f, gAspect); // Define the scenes camera
	LightD light; // Define the scenes light
	Screen screen(gScreenWidth, gScreenHeight, camera, light); // Define the
																														 // screen of the
																														 // scene
	Cube cube; // Library comes with one default mesh
	cube.translate(0.0f, 0.0f, -6.f); // Must translate the mesh away from the
																		// camera (-z is into the screen)'
	/*float deg = 0.1f;*/
	while(1){
		struct timespec time, time2;
		time.tv_sec = 0;
		time.tv_nsec = 5 * 100 * 1000 * 1000;
		nanosleep(&time, &time2);

		size_t count = 0;
		while((c = fgetc(sensor_file)) != EOF){
			if(count > 1028*10){
				count = 0;
			}

			//printf("c: %i\n", c);

			sensor_buffer[count] = c;
			count++;
		}
		//printf("count: %i ? %i\n", count, c);
		sensor_buffer[count + 1] = '\0';
		clearerr(sensor_file);

		count = 0;
		while(sensor_buffer[count] != '\0'){
			if(strncmp(sensor_buffer + count, "}\n}", 3) == 0){
				break ;
			}

			//printf("s: %.2s\n", sensor_buffer + count);

			count++;
		}
		if(strncmp(sensor_buffer + count, "}\n}", 3) != 0){
			continue ;
		}

		sensor_buffer[count + 3] = '\0';

		snprintf(sensor_json, 1028*10, "%s", sensor_buffer);
		//printf("-- debug --\n%s\n-- --\n", sensor_json);
		//printf("%s\n", sensor_json);
		yyjson_doc *doc = yyjson_read(sensor_json, strlen(sensor_json), 0);
		yyjson_val *root = yyjson_doc_get_root(doc);

		double scans[20] = { 0 };
		size_t idx, max;
		yyjson_val *key, *val;
		yyjson_obj_foreach(root, idx, max, key, val) {
			yyjson_val *values = yyjson_obj_get(val, "values");
			//printf("%i\n", idx);
			if(values == NULL)
				continue ;

			size_t idxb, maxb;
			yyjson_val *valb;
			yyjson_arr_foreach(values, idxb, maxb, valb) {
				scans[idxb] = yyjson_get_real(valb);
			}
		}

		/*printf("%f / %f / %f / %f\n",
			scans[0] * 180/3.14,
			scans[1] * 180/3.14,
			scans[2] * 180/3.14,
			scans[3] * 180/3.14
		);*/

		glm::quat quat = glm::quat(scans[0],
			scans[1],
			scans[2],
			scans[3]
		);

		glm::mat4 m = mat4_cast(quat);

		float y, p, r;
		glm::extractEulerAngleYXZ(m, y, p, r);
		y = y * 180/3.14;
		p = p * 180/3.14;
		r = r * 180/3.14;

		//printf("%f %f %f\n", y, p, r);
		screen.start();
		cube.rotate(p, y, 0.0f);

		screen.shadeMesh(cube);

		printf("\033[2J\033[H");
		fflush(stdout);
		screen.print();
		screen.clear();
		/**/

		//sleep(1);

		/*printf("[1] %f\n[2] %f\n[3] %f\n[4] %f\n[5]%f\n\n",
			scans[0],
			scans[1],
			scans[2],
			scans[3],
			scans[4]
		);*/
	}

	return 0;
}
