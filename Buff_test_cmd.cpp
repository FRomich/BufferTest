#include <iostream>

#include "CameraManager.h"
#include "FrameProcessor.h"
#include "DataBase.h"


int main()
{
	DataBase db;

	const char* conninfo =
		"host=172.29.166.213 "
		"port=5432 "
		"dbname=postgres "
		"user=postgres "
		"password=postgres "
		"connect_timeout=5";

	if (!db.connect(conninfo))
	{
		std::cerr << db.lastError() << '\n';
		return 1;
	}

	std::cout << "Connected\n";



#if 1
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

	const auto now = std::chrono::system_clock::now();
	const auto time =
		std::chrono::time_point_cast<std::chrono::seconds>(now);

	const auto sessionDirectory =
		std::filesystem::path("frames") /
		std::format("{:%Y-%m-%d_%H-%M-%S}", time);
	/////
	const std::string sessionName =
		sessionDirectory.filename().string();

	const auto sessionId = db.createSession(
		sessionDirectory.filename().string(),
		std::filesystem::absolute(sessionDirectory).string());

	if (sessionId == 0)
	{
		std::cerr << "Failed to create session: "
			<< db.lastError() << '\n';
		return EXIT_FAILURE;
	}

	std::cout << "Session ID: " << sessionId << '\n';


	for (size_t i = 0; i < cameras.count(); ++i)
	{
		auto* camera = cameras.camera(i);

		auto processor = std::make_unique<FrameProcessor>(
			camera->buffer(),
			db,
			sessionId,              // ID записи в frame_sessions
			static_cast<int>(i),    // ID/индекс камеры
			sessionDirectory /
			("camera_" + std::to_string(i)));

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
		std::chrono::seconds(5));

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
#endif

	return EXIT_SUCCESS;
}
