#include <framework.h>

class window: public koku::opengl::windowCallback
{
	private:
		koku::opengl::window my_window;
		bool run;

	protected:
		void onQuit()
		{
			run = false;
		}

	public:
		window():
			my_window(this, "koku-yuv4mpeg-3D", 640, 480, true),
			run(true)
		{}

		bool update()
		{
			my_window.update();
			//render stuff and co
			my_window.flip();
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
