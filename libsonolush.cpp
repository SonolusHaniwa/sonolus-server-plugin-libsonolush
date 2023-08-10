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

void initBuild(int argc, char** argv) {
    string path = argv[3], type = argv[2]; string extendCommand = "";
    for (int i = 4; i < argc; i++) extendCommand += " " + string(argv[i]);
    stringstream command;
    command << "cd \"" << path << "\"";
    command << " && g++ main.cpp -o main -ljsoncpp -lssl -lcrypto -lz "
            << (type == "play" ? "-Dplay" : "-Dtutorial") << " " << extendCommand;
    command << " && ./main";
    int res = system(command.str().c_str());
    if (res) exit(3);

    preload();
    string package_json = readFile((path + "/package.json").c_str());
    Json::Value arr; json_decode(package_json, arr);

    string engineData = uploadFile((path + "/dist/EngineData").c_str());
    string engineConfiguration = uploadFile((path + "/dist/EngineConfiguration").c_str());
	string engineTutorialData = uploadFile((path + "/dist/EngineTutorialData").c_str());
    string engineThumbnail = fileExist((path + "/dist/thumbnail.jpg").c_str()) ?
        uploadFile((path + "/dist/thumbnail.jpg").c_str()) : uploadFile((path + "/dist/thumbnail.png").c_str()); 

    for (int i = 0; i < arr["i18n"].size(); i++) {
        auto item = arr["i18n"][i];
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
        engineCreate(EngineItem(-1, arr["name"].asString(), item["title"].asString(), item["subtitle"].asString(), item["author"].asString(), 
            skin, background, effect, particle, SRL<EngineThumbnail>(engineThumbnail, ""), SRL<EngineData>(engineData, ""), 
            SRL<EngineConfiguration>(engineConfiguration, ""), SRL<EngineRom>("", ""), {engineTutorialData, ""}, item["description"].asString()), item["localization"].asString());
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
        return "v1.1.0";
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
            "Sonolus.h build: " + string(argv[0]) + " buildcpp [name] <play/tutorial> [args]"
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
