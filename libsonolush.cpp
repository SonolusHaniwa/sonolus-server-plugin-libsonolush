#include"../../main.cpp"
#include<bits/stdc++.h>
using namespace std;

void copyFolder(string src, string dst) {
    MKDIR(dst);
    for (auto& p: filesystem::directory_iterator(src)) {
        if (filesystem::is_directory(p)) copyFolder(p.path().string(), dst + "/" + p.path().filename().string());
        else filesystem::copy_file(p.path().string(), dst + "/" + p.path().filename().string());
    }
}

void initCustomEngine(char** argv) {
    string root_dir = string(argv[2]);
    copyFolder("./plugins/libsonolush/source", root_dir);
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
    ofstream fout(("./data/" + buffer.str()).c_str());
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

void initBuild(int argc, char** argv) {
    string path = argv[3], type = argv[2]; string extendCommand = "";
    for (int i = 4; i < argc; i++) extendCommand += " " + string(argv[i]);
    stringstream command;
    command << "cd \"" << path << "\"";
	command << " && echo [INFO] Compiling " << (type == "particle" ? "particle" : "engine") << " \"" << path << "\"...";
    command << " && g++ main.cpp -o main -ljsoncpp -lssl -lcrypto -lz -lpng ";
	if (type == "play") command << "-Dplay";
	else if (type == "tutorial") command << "-Dtutorial";
	else if (type == "preview") command << "-Dpreview";
	else if (type == "watch") command << "-Dwatch";
	// else if (type == "particle") command << "-Dparticle";
	else throw runtime_error("Unknown Compilation Type");
	command << " " << extendCommand;
	command << " && echo [INFO] Running scripts...";
    command << " && ./main";
    int res = system(command.str().c_str());
    if (res) exit(3);

    preload();
    string package_json = readFile((path + "/package.json").c_str());
    Json::Value arr; json_decode(package_json, arr);

    string engineData = uploadFile((path + "/dist/EngineData").c_str());
	string engineDataSize = getFileSize((path + "/dist/EngineData").c_str());
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

	string particleThumbnail = engineThumbnail;
	string particleThumbnailSize = engineThumbnailSize;
	string particleData = uploadFile((path + "/dist/ParticleData").c_str());
	string particleDataSize = getFileSize((path + "/dist/ParticleData").c_str());
	string particleTexture = uploadFile((path + "/dist/ParticleTexture").c_str());
	string particleTextureSize = getFileSize((path + "/dist/ParticleTexture").c_str());

	cout << "===========================================" << endl
		 << "Collected Resource Info: " << endl
		 << endl
		 << "Engine Thumbnail: " << engineThumbnail << engineThumbnailSize << endl
		 << "Engine Play Data: " << engineData << engineDataSize << endl
		 << "Engine Tutorial Data: " << engineTutorialData << engineTutorialDataSize << endl
		 << "Engine Preview Data: " << enginePreviewData << enginePreviewDataSize << endl
		 << "Engine Watch Data: " << engineWatchData << engineWatchDataSize << endl
		 << endl
		 << "Skin Thumbnail: " << skinThumbnail << skinThumbnailSize << endl
		 << "Skin Data: " << skinData << skinDataSize << endl
		 << "Skin Texture: " << skinTexture << skinTextureSize << endl
		 << endl
		 << "Particle Thumbnail: " << particleThumbnail << particleThumbnailSize << endl
		 << "Particle Data: " << particleData << particleDataSize << endl
		 << "Particle Texture: " << particleTexture << particleTextureSize << endl
		 << "===========================================" << endl;

    if (arr["engine"]["name"].asString() != "") for (int i = 0; i < arr["engine"]["i18n"].size(); i++) {
        auto item = arr["engine"]["i18n"][i];
        SkinItem skin; BackgroundItem background; EffectItem effect; ParticleItem particle;
        auto tmp = skinList("name = \"" + item["skin"].asString() + "\"");
        if (tmp.items.size() == 0) writeLog(LOG_LEVEL_ERROR, "Failed to find skin \"" + item["skin"].asString() + "\""), exit(0);
        skin = tmp.items[0];
        auto tmp2 = backgroundList("name = \"" + item["background"].asString() + "\"");
        if (tmp2.items.size() == 0) writeLog(LOG_LEVEL_ERROR, "Failed to find background \"" + item["background"].asString() + "\""), exit(0);
        background = tmp2.items[0];
        auto tmp3 = effectList("name = \"" + item["effect"].asString() + "\"");
        if (tmp3.items.size() == 0) writeLog(LOG_LEVEL_ERROR, "Failed to find effect \"" + item["effect"].asString() + "\""), exit(0);
        effect = tmp3.items[0];
        auto tmp4 = particleList("name = \"" + item["particle"].asString() + "\"");
        if (tmp4.items.size() == 0) writeLog(LOG_LEVEL_ERROR, "Failed to find particle \"" + item["particle"].asString() + "\""), exit(0);
        particle = tmp4.items[0];
        engineCreate(EngineItem(-1, arr["engine"]["name"].asString(), item["title"].asString(), item["subtitle"].asString(), item["author"].asString(), 
            skin, background, effect, particle, SRL<EngineThumbnail>(engineThumbnail, ""), SRL<EngineData>(engineData, ""), SRL<EngineTutorialData>(engineTutorialData, ""), SRL<EnginePreviewData>(enginePreviewData, ""), SRL<EngineWatchData>(engineWatchData, ""),
            SRL<EngineConfiguration>(engineConfiguration, ""), SRL<EngineRom>("", ""), item["description"].asString()), item["localization"].asString());
    }

	if (arr["skin"]["name"].asString() != "") for (int i = 0; i < arr["skin"]["i18n"].size(); i++) {
			auto item = arr["skin"]["i18n"][i];
			skinCreate(SkinItem(-1, arr["skin"]["name"].asString(), item["title"].asString(), item["subtitle"].asString(), item["author"].asString(),
				SRL<SkinThumbnail>(skinThumbnail, ""), SRL<SkinData>(skinData, ""), SRL<SkinTexture>(skinTexture, ""), item["description"].asString()), item["localization"].asString());
		}
	
	if (arr["particle"]["name"].asString() != "") for (int i = 0; i < arr["particle"]["i18n"].size(); i++) {
		auto item = arr["particle"]["i18n"][i];
		particleCreate(ParticleItem(-1, arr["particle"]["name"].asString(), item["title"].asString(), item["subtitle"].asString(), item["author"].asString(),
			SRL<ParticleThumbnail>(particleThumbnail, ""), SRL<ParticleData>(particleData, ""), SRL<ParticleTexture>(particleTexture, ""), item["description"].asString()), item["localization"].asString());
	}
}

class PluginSonolush: public SonolusServerPlugin {
    public:
    
    string onPluginName() const {
        return "Sonolus.h Plugin";
    }
    string onPluginDescription() const {
        return "C++ based Developer Toolkit for Sonolus";
    }
    string onPluginVersion() const {
        return "0.7.4";
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
            "Sonolus.h init: " + string(argv[0]) + " initcpp [name]",
            "Sonolus.h build: " + string(argv[0]) + " buildcpp <play/tutorial/preview/watch> [name] [args]"
        };
    }
    void onPluginRunner(int argc, char** argv) const {
        if (string(argv[1]) == "initcpp") {
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
