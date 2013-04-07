#include <framework.h>
#include <iostream>
#include <string>

const char* fragment_shader = R"glsl(

#version 400
layout(location = 0) out vec4 FragColor;
in vec2 pos_fg;
layout(origin_upper_left) in vec4 gl_FragCoord;

uniform sampler2D frame;
uniform int frame_mode_in;
uniform int frame_mode_out;
void main()
{
	FragColor = vec4(0,0,0,0);
	switch(frame_mode_out)
	{
		case 0:
			switch(frame_mode_in)
			{
				case 0:
					FragColor = texture2D(frame, pos_fg);
					break;
				case 1:
					FragColor = texture2D(frame, vec2(pos_fg.x/2, pos_fg.y));
					break;
				case 2:
					FragColor = texture2D(frame, vec2(pos_fg.x, pos_fg.y/2));
					break;
			}
			break;
		case 1:
			if (pos_fg.x < 0.5)
			{
				switch(frame_mode_in)
				{
					case 1:
						FragColor = texture2D(frame, pos_fg);
						break;
					case 2:
						FragColor = texture2D(frame, vec2(pos_fg.x*2, pos_fg.y/2));
						break;
				}
			}
			else
			{
				switch(frame_mode_in)
				{
					case 1:
						FragColor = texture2D(frame, pos_fg);
						break;
					case 2:
						FragColor = texture2D(frame, vec2(pos_fg.x*2, 0.5+pos_fg.y/2));
						break;
				}
			}
			break;
		case 2:
			if (pos_fg.y < 0.5)
			{
				switch(frame_mode_in)
				{
					case 1:
						FragColor = texture2D(frame, vec2(pos_fg.x/2.0, pos_fg.y*2.0));
						break;
					case 2:
						FragColor = texture2D(frame, pos_fg);
						break;
				}
			}
			else
			{
				switch(frame_mode_in)
				{
					case 1:
						FragColor = texture2D(frame, vec2(0.5+pos_fg.x/2.0, pos_fg.y*2.0));
						break;
					case 2:
						FragColor = texture2D(frame, pos_fg);
						break;
				}
			}
			break;
		case 3:
			if (mod(gl_FragCoord.y, 2.0) < 1.0)
			{
				switch(frame_mode_in)
				{
					case 1:
						FragColor = texture2D(frame, vec2(pos_fg.x/2.0, pos_fg.y));
						break;
					case 2:
						FragColor = texture2D(frame, vec2(pos_fg.x, pos_fg.y/2));
						break;
				}
			}
			else
			{
				switch(frame_mode_in)
				{
					case 1:
						FragColor = texture2D(frame, vec2(0.5+pos_fg.x/2.0, pos_fg.y));
						break;
					case 2:
						FragColor = texture2D(frame, vec2(pos_fg.x, 0.5+pos_fg.y/2));
						break;
				}
			}
			break;
		case 4:
			//not yet
			break;
	}

	//YUV TO RGB
	float y = FragColor.r;
	float u = FragColor.g - 0.5;
	float v = FragColor.b - 0.5;
	float r = y + 1.402 * v;
	float g = y - 0.344 * u - 0.714 * v;
	float b = y + 1.772 * u;
	FragColor = vec4(r,g,b,1);
}

)glsl";

using namespace std;

class window: public koku::opengl::windowCallback
{
	private:
		koku::opengl::window  my_window;
		koku::opengl::buffer  my_rect;
		koku::opengl::texture my_frame;
		koku::opengl::shader  my_shader;  //will take left/right and render it right
		koku::opengl::shader_uniform my_frame_sampler;
		koku::opengl::shader_uniform my_frame_mode_input;
		koku::opengl::shader_uniform my_frame_mode_output;
		koku::opengl::shader_uniform my_frame_ratio;

		/* FRAME MODES */
		//0 NONE
		//1 LEFT_RIGHT
		//2 TOP_BOTTOM
		//3 COL
		//4 ROW

		bool run;
		int frame_w;
		int frame_h;
		unsigned char *frame_data;
		unsigned char *frame_data_unpacked;

		int frame_mode_input;
		int frame_mode_output;

		float frame_ratio;

	protected:
		void onQuit()
		{
			run = false;
		}

		void onResize(int width, int height)
		{
			//reset
			glViewport(0, 0, width, height);

			frame_ratio = ((float(frame_w)/float(frame_h))*float(height))/float(width);
		}

	public:
		~window()
		{
			delete[] frame_data;
			delete[] frame_data_unpacked;
		}

		window(int frame_mode_input, int frame_mode_output):
			my_window(this, "koku-yuv4mpeg-3D", 640, 480, true, true),
			my_rect(&my_window),
			my_frame(&my_window),
			my_shader(&my_window),
			my_frame_sampler("frame"),
			my_frame_mode_input("frame_mode_in"),
			my_frame_mode_output("frame_mode_out"),
			my_frame_ratio("frame_ratio"),
			run(true),
			frame_w(-1),
			frame_h(-1),
			frame_data(0),
			frame_data_unpacked(0),
			frame_mode_input(frame_mode_input),
			frame_mode_output(frame_mode_output),
			frame_ratio(1)
		{
			const float vertex[]=
			{
				-1.0, -1.0,
				 1.0, -1.0,
				-1.0,  1.0,
				-1.0,  1.0,
				 1.0, -1.0,
				 1.0,  1.0
			};
			my_rect.upload(false, vertex, 6*2, 2);
			const unsigned short index[]=
			{
				0,1,2,3,4,5
			};
			my_rect.upload(true, index, 6, 1);

			my_shader.uploadVertex("#version 400\n"
								   "layout(location = 0) in vec2 pos;\n"
								   "out vec2 pos_fg;\n"
								   "uniform float frame_ratio;\n"
								   "void main()\n"
								   "{\n"
								   "	pos_fg = (pos+vec2(1.0,1.0))/2.0;\n"
								   "	pos_fg.y = 1.0-pos_fg.y;\n"
								   "	if (frame_ratio <= 1) \n"
								   "	gl_Position = vec4(pos.x*frame_ratio, pos.y, 0, 1);\n"
								   "	else\n"
								   "	gl_Position = vec4(pos.x, pos.y/frame_ratio, 0, 1);\n"
								   "}\n");

			my_shader.uploadFragment(fragment_shader);

			my_shader.compile();

			//upload dummy texture
			unsigned char* image = new unsigned char[16*16*3];

			for(int y = 0;  y < 16; ++y)
			for(int x = 0;  x < 16; ++x)
			{
				image[(x+y*16)*3+0] = y*(255/16);
				image[(x+y*16)*3+1] = x*(255/16);
				image[(x+y*16)*3+2] = ((x+y)/2)*(255/16);
			}

			my_frame.upload(image, 16, 16, 3);

			delete[] image;

			//get data from yuv4mpeg
			//setup
			char c;
			cin >> c;
			if (c != 'W')
			{
				cout << "YUV4MPEG2 File is wrong ! - Waiting for W" << endl;
				return;
			}
			cin >> frame_w;

			cin >> c;
			if (c != 'H')
			{
				cout << "YUV4MPEG2 File is wrong ! - Waiting for H" << endl;
				return;
			}
			cin >> frame_h;

			frame_data = new unsigned char[frame_w*frame_h*3/2];
			frame_data_unpacked = new unsigned char[frame_w*frame_h*3];

			onResize(640, 480);
		}

		bool update()
		{
			//get data from yuv4mpeg
			//wait for FRAME
			string data;
			while(data != "FRAME")
			{
				cin >> data;
				if (!cin.good())
				{
					return false;
				}
			}
			//read FRAME
			char ignore;
			cin.read(&ignore, 1); //first byte is nothing..
			cin.read(reinterpret_cast<char*>(frame_data), frame_w*frame_h*3/2);
			//convert packed to unpacked
			const unsigned char* data_y = frame_data;
			const unsigned char* data_u = frame_data+frame_w*frame_h;
			const unsigned char* data_v = frame_data+frame_w*frame_h+frame_w/2*frame_h/2;
			for(int y = 0; y < frame_h; ++y)
			for(int x = 0; x < frame_w; ++x)
			{
				frame_data_unpacked[(x+y*frame_w)*3+0] = data_y[x+y*frame_w]; //Y
				frame_data_unpacked[(x+y*frame_w)*3+1] = data_u[x/2+y/2*frame_w/2]; //U
				frame_data_unpacked[(x+y*frame_w)*3+2] = data_v[x/2+y/2*frame_w/2]; //V
			}
			//upload unpacked
			my_frame.upload(frame_data_unpacked, frame_w, frame_h, 3);
			//update window
			my_window.update();
			//render stuff and co
			my_window.begin();
				glClearColor(0,0,0,1);
				glClear(GL_COLOR_BUFFER_BIT);
				my_shader.begin();
					my_shader.set(&my_frame_sampler,  &my_frame);
					my_shader.set(&my_frame_mode_input, frame_mode_input);
					my_shader.set(&my_frame_mode_output, frame_mode_output);
					my_shader.set(&my_frame_ratio, frame_ratio);
					my_rect.render(3, 6);
				my_shader.end();
				my_window.flip();
			my_window.end();
			return run;
		}
};

int main(int argc, const char** argv)
{
	string data;
	if (argc > 2)
	{
		cin >> data;
	}

	if (data != "YUV4MPEG2")
	{
		cout << "You are using this programm wrong ! Use it this way:" << endl;
		cout << endl;
		cout << "MODE_INPUT can be:" << endl;
		cout << "	NONE" << endl;
		cout << "	LEFT_RIGHT" << endl;
		cout << "	TOP_BOTTOM" << endl;
		cout << "MODE_OUTPUT can be:" << endl;
		cout << "	NONE" << endl;
		cout << "	LEFT_RIGHT" << endl;
		cout << "	TOP_BOTTOM" << endl;
		cout << "	COL" << endl;
		cout << "	ROW" << endl;

		cout << endl << "Sample:" << endl;
		cout << " mplayer -vo yuv4mpeg:file=>(" << argv[0] << " LEFT_RIGHT ROW) \"very nice 3D video.wmv\"" << endl;
		cout << "	-> LEFT_RIGHT input  => left-side left-eye and right-side right-eye" << endl;
		cout << "	-> ROW        output => first row left-eye and second row right-eye" << endl;

		return 1;
	}

	int mode_in = 0, mode_out = 0;

	if (argc > 2)
	{
		string smode_in = argv[1];
		string smode_out = argv[2];

		if (smode_in == "NONE")
		{
			mode_in = 0;
		}
		else if (smode_in == "LEFT_RIGHT")
		{
			mode_in = 1;
		}
		else if (smode_in == "TOP_BOTTOM")
		{
			mode_in = 2;
		}
		else
		{
			//show helpfalse
			return main(1, argv);
		}

		if (smode_out == "NONE")
		{
			mode_out = 0;
		}
		else if (smode_out == "LEFT_RIGHT")
		{
			mode_out = 1;
		}
		else if (smode_out == "TOP_BOTTOM")
		{
			mode_out = 2;
		}
		else if (smode_out == "COL")
		{
			mode_out = 3;
		}
		else if (smode_out == "ROW")
		{
			mode_out = 4;
		}
		else
		{
			//show help
			return main(1, argv);
		}
	}
	else
	{
		return main(1, argv);
	}

	window my_window(mode_in, mode_out);
	while(my_window.update())
	{
		//wait for data
	}
	return 0;
}
