#define MKDIR MKDIR_decrypted
#include"../../main.cpp"
#undef MKDIR
#include<bits/stdc++.h>
using namespace std;

void MKDIR(string path) {
	filesystem::create_directories(path);
}

bool file_exists(string path) {
	return filesystem::exists(path);
}

void copyFolder(string src, string dst, set<string> except, string tmp = "") {
    MKDIR(dst);
    for (auto& p: filesystem::directory_iterator(src)) {
    	string path = tmp + "/" + p.path().filename().string();
    	if (except.find(path) != except.end() && file_exists(dst + "/" + p.path().filename().string())) continue;
        if (filesystem::is_directory(p)) copyFolder(p.path().string(), dst + "/" + p.path().filename().string(), except, path);
        else 
        	filesystem::copy_file(p.path().string(), dst + "/" + p.path().filename().string(), filesystem::copy_options::overwrite_existing),
        	cout << "Copied file from \"" << src+ "/" + p.path().filename().string() << "\" to \"" << dst+ "/" + p.path().filename().string() << "\"" << endl;
    }
}

void initCustomEngine(char** argv) {
    string root_dir = string(argv[2]);
	MKDIR(root_dir);
	MKDIR(root_dir + "/sonolus");
	filesystem::copy_file("./plugins/libsonolush/source/sonolus.h", root_dir + "/sonolus/sonolus.h", filesystem::copy_options::overwrite_existing);
	filesystem::copy_file("./plugins/libsonolush/source/main.cpp", root_dir + "/main.cpp", filesystem::copy_options::overwrite_existing);
}

void updateCustomEngine(char** argv) {
    string root_dir = string(argv[2]);
	MKDIR(root_dir + "/sonolus");
	filesystem::copy_file("./plugins/libsonolush/source/sonolus.h", root_dir + "/sonolus/sonolus.h", filesystem::copy_options::overwrite_existing);
}

string uploadFile(string path) {
    ifstream fin(path.c_str());
    fin.seekg(0, ios::end);
    int len = fin.tellg();
    if (len == -1) return "";
    fin.seekg(0, ios::beg);
    char* filePointerBeg = new char[len];
    fin.read(filePointerBeg, len);
    unsigned char* fileSha1 = sha1(filePointerBeg, len);
    stringstream buffer;
    for (int i = 0; i < 20; i++)
        buffer << hex << setw(2) << setfill('0') << int(fileSha1[i]);
    ofstream fout(("./data/" + buffer.str()).c_str(), ios::binary);
    fout.write(filePointerBeg, len); fout.close();
    free(filePointerBeg); free(fileSha1);
    return buffer.str();
}

string getFileSize(string path) {
	ifstream fin(path.c_str());
	fin.seekg(0, ios::end);
	double size = fin.tellg();
	stringstream ss;
	if (size < 2048) ss << fixed << setprecision(3) << "(" << size << "B)";
	else { size /= 1024.0; if (size < 2048) ss << fixed << setprecision(3) << "(" << size << "KB)";
	else { size /= 1024.0; if (size < 2048) ss << fixed << setprecision(3) << "(" << size << "MB)";
	else { size /= 1024.0; if (size < 2048) ss << fixed << setprecision(3) << "(" << size << "GB)";
	else { size /= 1024.0; ss << fixed << setprecision(3) << "(" << size << "TB)"; }}}}
	return ss.str();
}

void syncRepository() {
	MKDIR("./plugins/libsonolush/source");
	cout << "Fetching latest Sonolus.h..." << endl;
	string json = get_url("https://cors.littleyang.icu/https://api.github.com/repos/SonolusHaniwa/sonolus.h/releases/latest", 5);
	Json::Value arr; json_decode(json, arr);
	cout << "Latest version: " << arr["tag_name"].asString() << endl;
	cout << "Downloading template.zip..." << endl;
	string zipball = get_url("https://cors.littleyang.icu/" + arr["zipball_url"].asString(), 5);
	string zipPath = "./plugins/libsonolush/source/template.zip";
	ofstream fout(zipPath, ios::binary);
	fout.write(zipball.c_str(), zipball.size()); fout.close();
	cout << "Downloaded template.zip for " << arr["tag_name"].asString() << getFileSize(zipPath) << endl;
	vector<string> files = getFileListFromZip(zipPath);
	for (int i = 0; i < files.size(); i++) {
		if (files[i].back() == '/') continue;
		string path = files[i];
		path = path.substr(path.find("/") + 1);
		if (path.find("/") != string::npos) {
			string dir = path.substr(0, path.rfind("/"));
			MKDIR("./plugins/libsonolush/source/" + dir);
		}
		string content = getFileFromZip(zipPath, files[i]);
		ofstream fout("./plugins/libsonolush/source/" + path, ios::binary);
		fout.write(content.c_str(), content.size());
		fout.close();
		cout << "Extracted \"" << path << "\" from template.zip" << endl;
	}
	filesystem::remove(zipPath);
	cout << "Compiling compiler..." << endl;
	system("g++ ./plugins/libsonolush/source/compiler/main.cpp -o ./plugins/libsonolush/source/compiler/main -O3 -g -w");
	cout << "Sync finished. Current Sonolus.h version: " << arr["tag_name"].asString() << endl;
}

void initBuild(int argc, char** argv) {
    string path = argv[3], type = argv[2]; string extendCommand = "";
    for (int i = 4; i < argc; i++) extendCommand += " " + string(argv[i]);
    stringstream command;
    command << "cd \"" << path << "\"";
	command << " && echo Compiling " << (type == "particle" ? "particle" : "engine") << " \"" << path << "\"...";
    command << " && mkdir .sonolus -p && ../plugins/libsonolush/source/compiler/main main.cpp .sonolus && g++ .sonolus/main.cpp -o main -g -w -fpermissive -ljsoncpp -lz -lpng -lzip ";
	if (type == "play") command << "-Dplay";
	else if (type == "tutorial") command << "-Dtutorial";
	else if (type == "preview") command << "-Dpreview";
	else if (type == "watch") command << "-Dwatch";
	else if (type == "all") ;
	// else if (type == "particle") command << "-Dparticle";
	else throw runtime_error("Unknown Compilation Type");
	command << " " << extendCommand;
	command << " && echo Running scripts...";
    command << " && ./main";
	if (type == "all") {
		command.str("");
		command << "cd \"" << path << "\"";
		command << " && echo Interpreting your code...";
		command << " && ../plugins/libsonolush/source/compiler/main main.cpp .sonolus";
		command << " && echo Compiling play mode of engine \"" << path << "\"...";
		command << " && g++ .sonolus/main.cpp -o main -g -w -fpermissive -ljsoncpp -lz -lpng -lzip -Dplay && ./main";
		command << " && echo Compiling tutorial mode of engine \"" << path << "\"...";
		command << " && g++ .sonolus/main.cpp -o main -g -w -fpermissive -ljsoncpp -lz -lpng -lzip -Dtutorial && ./main";
		command << " && echo Compiling preview mode of engine \"" << path << "\"...";
		command << " && g++ .sonolus/main.cpp -o main -g -w -fpermissive -ljsoncpp -lz -lpng -lzip -Dpreview && ./main";
		command << " && echo Compiling watch mode of engine \"" << path << "\"...";
		command << " && g++ .sonolus/main.cpp -o main -g -w -fpermissive -ljsoncpp -lz -lpng -lzip -Dwatch && ./main";
	}
    int res = system(command.str().c_str());
    if (res) exit(3);

    preload();
    string package_json = readFile((path + "/package.json").c_str());
    Json::Value arr; json_decode(package_json, arr);

	if (arr["level"]["generate"].asString() != "") {
		cout << "Generating LevelData..." << endl;
		int res = system(("cd \"" + path + "\" && ./main " + arr["level"]["generate"].asString()).c_str());
		if (res) exit(3);
	}

    string engineData = uploadFile((path + "/dist/EnginePlayData").c_str());
	string engineDataSize = getFileSize((path + "/dist/EnginePlayData").c_str());
    string engineConfiguration = uploadFile((path + "/dist/EngineConfiguration").c_str());
	string engineConfigurationSize = getFileSize((path + "/dist/EngineConfiguration").c_str());
	string engineTutorialData = uploadFile((path + "/dist/EngineTutorialData").c_str());
	string engineTutorialDataSize = getFileSize((path + "/dist/EngineTutorialData").c_str());
	string enginePreviewData = uploadFile((path + "/dist/EnginePreviewData").c_str());
	string enginePreviewDataSize = getFileSize((path + "/dist/EnginePreviewData").c_str());
	string engineWatchData = uploadFile((path + "/dist/EngineWatchData").c_str());
	string engineWatchDataSize = getFileSize((path + "/dist/EngineWatchData").c_str());
    string engineThumbnail = fileExist((path + "/dist/thumbnail.jpg").c_str()) ?
        uploadFile((path + "/dist/thumbnail.jpg").c_str()) : uploadFile((path + "/dist/thumbnail.png").c_str());
	string engineThumbnailSize = fileExist((path + "/dist/thumbnail.jpg").c_str()) ?
		getFileSize((path + "/dist/thumbnail.jpg").c_str()) : getFileSize((path + "/dist/thumbnail.png").c_str());

	string skinThumbnail = engineThumbnail;
	string skinThumbnailSize = engineThumbnailSize;
	string skinData = uploadFile((path + "/dist/SkinData").c_str());
	string skinDataSize = getFileSize((path + "/dist/SkinData").c_str());
	string skinTexture = uploadFile((path + "/dist/SkinTexture").c_str());
	string skinTextureSize = getFileSize((path + "/dist/SkinTexture").c_str());

	string backgroundThumbnail = engineThumbnail;
	string backgroundThumbnailSize = engineThumbnailSize;
	string backgroundData = uploadFile((path + "/dist/BackgroundData").c_str());
	string backgroundDataSize = getFileSize((path + "/dist/BackgroundData").c_str());
	string backgroundImage = uploadFile((path + "/dist/BackgroundImage").c_str());
	string backgroundImageSize = getFileSize((path + "/dist/BackgroundImage").c_str());
	string backgroundConfiguration = uploadFile((path + "/dist/BackgroundConfiguration").c_str());
	string backgroundConfigurationSize = getFileSize((path + "/dist/BackgroundConfiguration").c_str());

    string effectThumbnail = engineThumbnail;
	string effectThumbnailSize = engineThumbnailSize;
	string effectData = uploadFile((path + "/dist/EffectData").c_str());
	string effectDataSize = getFileSize((path + "/dist/EffectData").c_str());
	string effectAudio = uploadFile((path + "/dist/EffectAudio").c_str());
	string effectAudioSize = getFileSize((path + "/dist/EffectAudio").c_str());

	string particleThumbnail = engineThumbnail;
	string particleThumbnailSize = engineThumbnailSize;
	string particleData = uploadFile((path + "/dist/ParticleData").c_str());
	string particleDataSize = getFileSize((path + "/dist/ParticleData").c_str());
	string particleTexture = uploadFile((path + "/dist/ParticleTexture").c_str());
	string particleTextureSize = getFileSize((path + "/dist/ParticleTexture").c_str());

	string levelCover = engineThumbnail;
	string levelCoverSize = engineThumbnailSize;
	string levelBgm = uploadFile((path + "/dist/LevelBgm").c_str());
	string levelBgmSize = getFileSize((path + "/dist/LevelBgm").c_str());
	string levelData = uploadFile((path + "/dist/LevelData").c_str());
	string levelDataSize = getFileSize((path + "/dist/LevelData").c_str());
	string levelPreview = uploadFile((path + "/dist/LevelPreview").c_str());
	string levelPreviewSize = getFileSize((path + "/dist/LevelPreview").c_str());

	cout << "===========================================" << endl
		 << "Collected Resource Info: " << endl
		 << endl
		 << "Engine Thumbnail: " << engineThumbnail << engineThumbnailSize << endl
		 << "Engine Configuration: " << engineConfiguration << engineConfigurationSize << endl
		 << "Engine Play Data: " << engineData << engineDataSize << endl
		 << "Engine Tutorial Data: " << engineTutorialData << engineTutorialDataSize << endl
		 << "Engine Preview Data: " << enginePreviewData << enginePreviewDataSize << endl
		 << "Engine Watch Data: " << engineWatchData << engineWatchDataSize << endl
		 << endl
		 << "Skin Thumbnail: " << skinThumbnail << skinThumbnailSize << endl
		 << "Skin Data: " << skinData << skinDataSize << endl
		 << "Skin Texture: " << skinTexture << skinTextureSize << endl
		 << endl
		 << "Background Thumbnail: " << backgroundThumbnail << backgroundThumbnailSize << endl
		 << "Background Data: " << backgroundData << backgroundDataSize << endl
		 << "Background Image: " << backgroundImage << backgroundImageSize << endl
		 << "Background Configuration: " << backgroundConfiguration << backgroundConfigurationSize << endl
		 << endl
		 << "Effect Thumbnail: " << effectThumbnail << effectThumbnailSize << endl
		 << "Effect Data: " << effectData << effectDataSize << endl
		 << "Effect Audio: " << effectAudio << effectAudioSize << endl
		 << endl
		 << "Particle Thumbnail: " << particleThumbnail << particleThumbnailSize << endl
		 << "Particle Data: " << particleData << particleDataSize << endl
		 << "Particle Texture: " << particleTexture << particleTextureSize << endl
		 << endl
		 << "Level Cover: " << levelCover << levelCoverSize << endl
		 << "Level Bgm: " << levelBgm << levelBgmSize << endl
		 << "Level Data: " << levelData << levelDataSize << endl
		 << "Level Preview: " << levelPreview << levelPreviewSize << endl
		 << "===========================================" << endl;

	if (arr["skin"]["name"].asString() != "") for (int i = 0; i < arr["skin"]["i18n"].size(); i++) {
			auto item = arr["skin"]["i18n"][i];
			skinsCreate(SkinItem(-1, arr["skin"]["name"].asString(), item["title"].asString(), item["subtitle"].asString(), item["author"].asString(),
				SRL<SkinThumbnail>(skinThumbnail, ""), SRL<SkinData>(skinData, ""), SRL<SkinTexture>(skinTexture, ""), {}, item["description"].asString()), item["localization"].asString());
		}
	
	if (arr["background"]["name"].asString() != "") for (int i = 0; i < arr["background"]["i18n"].size(); i++) {
			auto item = arr["background"]["i18n"][i];
			backgroundsCreate(BackgroundItem(-1, arr["background"]["name"].asString(), item["title"].asString(), item["subtitle"].asString(), item["author"].asString(),
				SRL<BackgroundThumbnail>(backgroundThumbnail, ""), SRL<BackgroundData>(backgroundData, ""), SRL<BackgroundImage>(backgroundImage, ""), SRL<BackgroundConfiguration>(backgroundConfiguration, ""), {}, item["description"].asString()), item["localization"].asString());
		}

	if (arr["effect"]["name"].asString() != "") for (int i = 0; i < arr["effect"]["i18n"].size(); i++) {
			auto item = arr["effect"]["i18n"][i];
			effectsCreate(EffectItem(-1, arr["effect"]["name"].asString(), item["title"].asString(), item["subtitle"].asString(), item["author"].asString(),
				SRL<EffectThumbnail>(effectThumbnail, ""), SRL<EffectData>(effectData, ""), SRL<EffectAudio>(effectAudio, ""), {}, item["description"].asString()), item["localization"].asString());
		}
	
	if (arr["particle"]["name"].asString() != "") for (int i = 0; i < arr["particle"]["i18n"].size(); i++) {
		auto item = arr["particle"]["i18n"][i];
		particlesCreate(ParticleItem(-1, arr["particle"]["name"].asString(), item["title"].asString(), item["subtitle"].asString(), item["author"].asString(),
			SRL<ParticleThumbnail>(particleThumbnail, ""), SRL<ParticleData>(particleData, ""), SRL<ParticleTexture>(particleTexture, ""), {}, item["description"].asString()), item["localization"].asString());
	}

    if (arr["engine"]["name"].asString() != "") for (int i = 0; i < arr["engine"]["i18n"].size(); i++) {
        auto item = arr["engine"]["i18n"][i];
        SkinItem skin; BackgroundItem background; EffectItem effect; ParticleItem particle;
        auto tmp = skinsList("name = \"" + item["skin"].asString() + "\"", "");
        if (tmp.size() == 0) writeLog(LOG_LEVEL_ERROR, "Failed to find skin \"" + item["skin"].asString() + "\""), exit(0);
        skin = tmp[0];
        auto tmp2 = backgroundsList("name = \"" + item["background"].asString() + "\"", "");
        if (tmp2.size() == 0) writeLog(LOG_LEVEL_ERROR, "Failed to find background \"" + item["background"].asString() + "\""), exit(0);
        background = tmp2[0];
        auto tmp3 = effectsList("name = \"" + item["effect"].asString() + "\"", "");
        if (tmp3.size() == 0) writeLog(LOG_LEVEL_ERROR, "Failed to find effect \"" + item["effect"].asString() + "\""), exit(0);
        effect = tmp3[0];
        auto tmp4 = particlesList("name = \"" + item["particle"].asString() + "\"", "");
        if (tmp4.size() == 0) writeLog(LOG_LEVEL_ERROR, "Failed to find particle \"" + item["particle"].asString() + "\""), exit(0);
        particle = tmp4[0];
        enginesCreate(EngineItem(-1, arr["engine"]["name"].asString(), item["title"].asString(), item["subtitle"].asString(), item["author"].asString(), 
            skin, background, effect, particle, SRL<EngineThumbnail>(engineThumbnail, ""), SRL<EngineData>(engineData, ""), SRL<EngineTutorialData>(engineTutorialData, ""), SRL<EnginePreviewData>(enginePreviewData, ""), SRL<EngineWatchData>(engineWatchData, ""),
            SRL<EngineConfiguration>(engineConfiguration, ""), {}, SRL<EngineRom>("", ""), item["description"].asString()), item["localization"].asString());
    }

	if (arr["level"]["name"].asString() != "" && arr["level"]["generate"].asString() != "") for (int i = 0; i < arr["level"]["i18n"].size(); i++) {
		auto item = arr["level"]["i18n"][i];
		EngineItem engine; UseItem<SkinItem> skin; UseItem<BackgroundItem> background; UseItem<EffectItem> effect; UseItem<ParticleItem> particle;
		auto tmp = enginesList("name = \"" + item["engine"].asString() + "\"", "");
		if (tmp.size() == 0) writeLog(LOG_LEVEL_ERROR, "Failed to find engine \"" + item["engine"].asString() + "\""), exit(0);
		engine = tmp[0];
		if (item["skin"].asString() == "") skin.useDefault = true;
		else {
			auto tmp2 = skinsList("name = \"" + item["skin"].asString() + "\"", "");
			if (tmp2.size() == 0) writeLog(LOG_LEVEL_ERROR, "Failed to find skin \"" + item["skin"].asString() + "\""), exit(0);
			skin.item = tmp2[0]; skin.useDefault = false;
		}
		if (item["background"].asString() == "") background.useDefault = true;
		else {
			auto tmp3 = backgroundsList("name = \"" + item["background"].asString() + "\"", "");
			if (tmp3.size() == 0) writeLog(LOG_LEVEL_ERROR, "Failed to find background \"" + item["background"].asString() + "\""), exit(0);
			background.item = tmp3[0]; background.useDefault = false;
		}
		if (item["effect"].asString() == "") effect.useDefault = true;
		else {
			auto tmp4 = effectsList("name = \"" + item["effect"].asString() + "\"", "");
			if (tmp4.size() == 0) writeLog(LOG_LEVEL_ERROR, "Failed to find effect \"" + item["effect"].asString() + "\""), exit(0);
			effect.item = tmp4[0]; effect.useDefault = false;
		}
		if (item["particle"].asString() == "") particle.useDefault = true;
		else {
			auto tmp5 = particlesList("name = \"" + item["particle"].asString() + "\"", "");
			if (tmp5.size() == 0) writeLog(LOG_LEVEL_ERROR, "Failed to find particle \"" + item["particle"].asString() + "\""), exit(0);
			particle.item = tmp5[0]; particle.useDefault = false;
		}
		levelsCreate(LevelItem(-1, arr["level"]["name"].asString(), item["rating"].asInt(), item["title"].asString(), item["artists"].asString(), item["author"].asString(), engine, skin, background, effect, particle,
			SRL<LevelCover>(levelCover, ""), SRL<LevelBgm>(levelBgm, ""), SRL<LevelData>(levelData, ""), SRL<LevelPreview>(levelPreview, ""), {}, item["description"].asString()), item["localization"].asString());
	}
}

class PluginSonolush: public SonolusServerPlugin {
    public:
    
    string onPluginName() const {
        return "Sonolus.h Plugin";
    }
    string onPluginDescription() const {
        return "C++ Based Developer Toolkit for Sonolus";
    }
    string onPluginVersion() const {
        return "1.0.0";
    }
    string onPluginPlatformVersion() const {
        return sonolus_server_version;
    }
    string onPluginAuthor() const {
        return "LittleYang0531";
    }
    string onPluginLicense() const {
        return "MIT";
    }
    string onPluginWebsite() const {
        return "https://github.com/SonolusHaniwa/sonolus.h";
    }
    vector<string> onPluginHelp(char** argv) const {
        return {
            "Sonolus.h sync: " + string(argv[0]) + " synccpp",
            "Sonolus.h update: " + string(argv[0]) + " updatecpp [name]",
            "Sonolus.h init: " + string(argv[0]) + " initcpp [name]",
            "Sonolus.h build: " + string(argv[0]) + " buildcpp <play/tutorial/preview/watch/all> [name] [args]"
        };
    }
    void onPluginRunner(int argc, char** argv) const {
    	if (argc < 2) return;
    	if (string(argv[1]) == "synccpp") {
    		syncRepository();
    		exit(0);
    	} else if (string(argv[1]) == "updatecpp") {
            if (argc < 3) return;
            updateCustomEngine(argv);
            exit(0);
    	} else if (string(argv[1]) == "initcpp") {
            if (argc < 3) return;
            initCustomEngine(argv);
            exit(0);
        } else if (string(argv[1]) == "buildcpp") {
            if (argc < 4) return;
            initBuild(argc, argv);
            serverRunner(argc, argv);
            exit(0);
        } return;
    }
    void onPluginRouter(int argc, char** argv, application *app) const {
        
    }
};

PLUMA_INHERIT_PROVIDER(PluginSonolush, SonolusServerPlugin);

PLUMA_CONNECTOR
bool pluma_connect(pluma::Host& host) {
    host.add( new PluginSonolushProvider() );
    return true;
}
