#include <iostream>

#include "MvCameraControl.h"

#include "CameraManager.h"

int main()
{
	CameraManager cameras;

	if (!cameras.initialize())
		return 1;

	std::cout << "Found cameras: "
		<< cameras.count()
		<< '\n';

	for (size_t i = 0; i < cameras.count(); ++i)
	{
		const auto* camera = cameras.camera(i);

		std::cout << '[' << i << "] "
			<< camera->modelName()
			<< " SN: "
			<< camera->serialNumber()
			<< '\n';
	}

	if (!cameras.openAll())
		return 1;

	if (!cameras.startAll())
		return 1;

	//посмотреть что с буфером

	cameras.stopAll();
	cameras.closeAll();

	return EXIT_SUCCESS;
}
