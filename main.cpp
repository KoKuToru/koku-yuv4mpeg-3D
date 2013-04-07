#include <framework.h>

class window: public koku::opengl::windowCallback
{
	private:
		koku::opengl::window  my_window;
		koku::opengl::buffer  my_rect;
		koku::opengl::texture my_eye_left;
		koku::opengl::texture my_eye_right;
		koku::opengl::shader  my_shader;  //will take left/right and render it right
		koku::opengl::shader_uniform my_eye_sampler_left;
		koku::opengl::shader_uniform my_eye_sampler_right;
		bool run;

	protected:
		void onQuit()
		{
			run = false;
		}

	public:
		window():
			my_window(this, "koku-yuv4mpeg-3D", 640, 480, true),
			my_rect(&my_window),
			my_eye_left(&my_window),
			my_eye_right(&my_window),
			my_shader(&my_window),
			my_eye_sampler_left("eye_left"),
			my_eye_sampler_right("eye_right"),
			run(true)
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
								   "	gl_Position = vec4(pos, 0, 1);\n"
								   "}\n");

			my_shader.uploadFragment("#version 400\n"
									 "layout(location = 0) out vec4 FragColor;\n"
									 "in vec2 pos_fg;\n"
									 "uniform sampler2D eye_left;\n"
									 "uniform sampler2D eye_right;\n"
									 "void main()\n"
									 "{\n"
									 "	FragColor = texture2D(eye_left, pos_fg);\n"
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

			my_eye_left.upload((unsigned char*)image, 16, 16, 3);
			my_eye_right.upload((unsigned char*)image, 16, 16, 3);

			delete image;
		}

		bool update()
		{
			my_window.update();
			//render stuff and co
			my_window.begin();
				glClearColor(1,1,1,1);
				glClear(GL_COLOR_BUFFER_BIT);
				my_shader.begin();
					my_shader.set(&my_eye_sampler_left,  &my_eye_left);
					my_shader.set(&my_eye_sampler_right, &my_eye_right);
					my_rect.render(3, 6);
				my_shader.end();
				my_window.flip();
			my_window.end();
			return run;
		}
};

int main()
{
	//does nothing for now
	window my_window;

	while(my_window.update())
	{
		//wait for data
	}
	return 0;
}
