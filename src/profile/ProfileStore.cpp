#include "rodavarion/profile/ProfileStore.hpp"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>
namespace rodavarion::profile {
QString ProfileStore::defaultFilePath(){auto d=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);QDir().mkpath(d);return QDir(d).filePath("profiles.json");}
core::Result<std::vector<Profile>> ProfileStore::load(const QString& path) const{
 QFile f(path); if(!f.exists()) return core::Result<std::vector<Profile>>::success({});
 if(!f.open(QIODevice::ReadOnly)) return core::Result<std::vector<Profile>>::failure("Cannot open profiles file.");
 QJsonParseError e; auto doc=QJsonDocument::fromJson(f.readAll(),&e); if(e.error!=QJsonParseError::NoError||!doc.isArray()) return core::Result<std::vector<Profile>>::failure("Invalid profiles file.");
 std::vector<Profile> out; for(const auto&v:doc.array()){if(!v.isObject())continue;auto o=v.toObject();Profile p;p.id=o.value("id").toString().toStdString();p.displayName=o.value("displayName").toString().toStdString();auto st=o.value("settings").toObject();for(auto it=st.begin();it!=st.end();++it)p.settings[it.key().toStdString()]=it.value().toString().toStdString();if(!p.id.empty())out.push_back(std::move(p));}
 return core::Result<std::vector<Profile>>::success(std::move(out));
}
core::Result<void> ProfileStore::save(const QString& path,const std::vector<Profile>& ps) const{
 QDir().mkpath(QFileInfo(path).absolutePath());QJsonArray a;for(const auto&p:ps){QJsonObject st;for(const auto&[k,v]:p.settings)st.insert(QString::fromStdString(k),QString::fromStdString(v));QJsonObject o;o.insert("id",QString::fromStdString(p.id));o.insert("displayName",QString::fromStdString(p.displayName));o.insert("settings",st);a.append(o);}QSaveFile f(path);if(!f.open(QIODevice::WriteOnly))return core::Result<void>::failure("Cannot write profiles file.");f.write(QJsonDocument(a).toJson(QJsonDocument::Indented));if(!f.commit())return core::Result<void>::failure("Cannot commit profiles file.");return core::Result<void>::success();
}
}
