#include <iostream>

#include "MvCameraControl.h"

#include "CameraManager.h"
#include "FrameProcessor.h"

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

	// Создаём обработчик для каждой камеры
	std::vector<std::unique_ptr<FrameProcessor>> processors;

	for (size_t i = 0; i < cameras.count(); ++i)
	{
		auto* camera = cameras.camera(i);

		auto processor = std::make_unique<FrameProcessor>(
			camera->buffer(),
			"frames/camera_" + std::to_string(i));

		if (!processor->start())
		{
			std::cerr
				<< "Failed to start processor for camera "
				<< i
				<< '\n';

			return 1;
		}

		processors.push_back(std::move(processor));
	}


	if (!cameras.startAll())
		return 1;

	// Работаем 30 секунд
	std::this_thread::sleep_for(
		std::chrono::seconds(30));

	cameras.stopAll();

	for (auto& processor : processors)
	{
		processor->stop();
	}

	// Статистика
	for (size_t i = 0; i < cameras.count(); ++i)
	{
		auto* camera = cameras.camera(i);
		const auto& processor = processors[i];

		std::cout
			<< "\nCamera " << i
			<< '\n';

		std::cout
			<< "  Processed: "
			<< processor->processed()
			<< '\n';

		std::cout
			<< "  Failed:    "
			<< processor->failed()
			<< '\n';

		std::cout
			<< "  Dropped:   "
			<< camera->buffer().dropped()
			<< '\n';
	}


	cameras.closeAll();

	return EXIT_SUCCESS;
}
