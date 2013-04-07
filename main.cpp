#include <framework.h>
#include <iostream>
#include <string>
using namespace std;


class window: public koku::opengl::windowCallback
{
	private:
		koku::opengl::window  my_window;
		koku::opengl::buffer  my_rect;
		koku::opengl::texture my_frame;
		koku::opengl::shader  my_shader;  //will take left/right and render it right
		koku::opengl::shader_uniform my_frame_sampler;
		bool run;
		int frame_w;
		int frame_h;
		unsigned char *frame_data;
		unsigned char *frame_data_unpacked;

	protected:
		void onQuit()
		{
			run = false;
		}

	public:
		window():
			my_window(this, "koku-yuv4mpeg-3D", 640, 480, true),
			my_rect(&my_window),
			my_frame(&my_window),
			my_shader(&my_window),
			my_frame_sampler("frame"),
			run(true),
			frame_w(-1),
			frame_h(-1),
			frame_data(0),
			frame_data_unpacked(0)
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
								   "void main()\n"
								   "{\n"
								   "	pos_fg = (pos+vec2(1.0,1.0))/2.0;\n"
								   "	pos_fg.y = 1.0-pos_fg.y;\n"
								   "	gl_Position = vec4(pos, 0, 1);\n"
								   "}\n");

			my_shader.uploadFragment("#version 400\n"
									 "layout(location = 0) out vec4 FragColor;\n"
									 "in vec2 pos_fg;\n"
									 "uniform sampler2D frame;\n"
									 "void main()\n"
									 "{\n"
									 "	FragColor = texture2D(frame, pos_fg);\n"
									 //YUV TO RGB
									 "	float y = FragColor.r;\n"
									 "	float u = FragColor.g - 0.5;\n"
									 "	float v = FragColor.b - 0.5;\n"
									 "	float r = y + 1.402 * v;\n"
									 "	float g = y - 0.344 * u - 0.714 * v;\n"
									 "	float b = y + 1.772 * u;\n"
									 "	FragColor = vec4(r,g,b,1);\n"
									 "}\n");
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

			delete image;
		}

		bool update()
		{
			//get data from yuv4mpeg
			if (frame_w == -1)
			{
				//setup
				char c;
				cin >> c;
				if (c != 'W')
				{
					cout << "YUV4MPEG2 File is wrong ! - Waiting for W" << endl;
					return false;
				}
				cin >> frame_w;

				cin >> c;
				if (c != 'H')
				{
					cout << "YUV4MPEG2 File is wrong ! - Waiting for H" << endl;
					return false;
				}
				cin >> frame_h;

				frame_data = new unsigned char[frame_w*frame_h*3/2];
				frame_data_unpacked = new unsigned char[frame_w*frame_h*3];
			}
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
				glClearColor(1,1,1,1);
				glClear(GL_COLOR_BUFFER_BIT);
				my_shader.begin();
					my_shader.set(&my_frame_sampler,  &my_frame);
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
	cin >> data;

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


	window my_window;
	while(my_window.update())
	{
		//wait for data
	}
	return 0;
}
