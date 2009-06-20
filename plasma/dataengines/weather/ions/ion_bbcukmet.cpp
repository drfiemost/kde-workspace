/***************************************************************************
 *   Copyright (C) 2007-2009 by Shawn Starr <shawn.starr@rogers.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

/* Ion for BBC's Weather from the UK Met Office */

#include "ion_bbcukmet.h"
#include "../../time/solarposition.h"

class UKMETIon::Private : public QObject
{
private:
    struct XMLMapInfo {
        QString place;
        QString XMLurl;
        QString XMLforecastURL;
        bool ukPlace;
    };

public:
    // Key dicts
    QHash<QString, UKMETIon::Private::XMLMapInfo> m_place;
    QVector<QString> m_locations;

    // Weather information
    QHash<QString, WeatherData> m_weatherData;

    // Store KIO jobs - Search list
    QMap<KJob *, QXmlStreamReader*> m_jobXml;
    QMap<KJob *, QString> m_jobList;

    QMap<KJob *, QXmlStreamReader*> m_obsJobXml;
    QMap<KJob *, QString> m_obsJobList;

    QMap<KJob *, QXmlStreamReader *> m_forecastJobXml;
    QMap<KJob *, QString> m_forecastJobList;

    KIO::TransferJob *m_job;

    QDateTime m_dateFormat;
};

static double calculateSunriseTime(const int & day, const int & month, const int & year, const double & latitude, const double & longitude)
{

    QDate d(year, month, day);
    QDateTime dt(d);

    double jd, century, eqTime, solarDec, azimuth, zenith;

    NOAASolarCalc::calc(dt, longitude, latitude, 0.0,
                        &jd, &century, &eqTime, &solarDec, &azimuth, &zenith);

    double toReturn;
    NOAASolarCalc::calcTimeUTC(zenith, true, &jd, &toReturn, latitude, longitude);

    toReturn *= 60;
    return toReturn;
}

static double calculateSunsetTime(const int & day, const int & month, const int & year, const double & latitude, const double & longitude)
{

    QDate d(year, month, day);
    QDateTime dt(d);

    double jd, century, eqTime, solarDec, azimuth, zenith;

    NOAASolarCalc::calc(dt, longitude, latitude, 0.0,
                        &jd, &century, &eqTime, &solarDec, &azimuth, &zenith);

    double toReturn;
    NOAASolarCalc::calcTimeUTC(zenith, false, &jd, &toReturn, latitude, longitude);

    toReturn *= 60;
    return toReturn;

}

// ctor, dtor
UKMETIon::UKMETIon(QObject *parent, const QVariantList &args)
        : IonInterface(parent, args), d(new Private())

{
    Q_UNUSED(args)
}

UKMETIon::~UKMETIon()
{
    // Destroy each forecast stored in a QVector
    foreach(const WeatherData &item, d->m_weatherData) {
        foreach(WeatherData::ForecastInfo *forecast, item.forecasts) {
            if (forecast) {
                delete forecast;
            }
        }
    }

    // Destroy dptr
    delete d;
}

void UKMETIon::reset()
{
    // Destroy each forecast stored in a QVector
    foreach(const WeatherData &item, d->m_weatherData) {
        foreach(WeatherData::ForecastInfo *forecast, item.forecasts) {
            if (forecast) {
                delete forecast;
            }
        }
    }

    // Destroy dptr
    delete d;
    d = new Private();
    emit(resetCompleted(this, true));
}



// Get the master list of locations to be parsed
void UKMETIon::init()
{
    setInitialized(true);
}

QMap<QString, IonInterface::ConditionIcons> UKMETIon::setupDayIconMappings(void)
{
//    ClearDay, FewCloudsDay, PartlyCloudyDay, Overcast,
//    Showers, ScatteredShowers, Thunderstorm, Snow,
//    FewCloudsNight, PartlyCloudyNight, ClearNight,
//    Mist, NotAvailable

    QMap<QString, ConditionIcons> dayList;
    dayList["sunny"] = ClearDay;
    //dayList["sunny night"] = ClearNight;
    dayList["clear"] = ClearDay;
    dayList["sunny intervals"] = PartlyCloudyDay;
    //dayList["sunny intervals night"] = ClearNight;
    dayList["partly cloudy"] = PartlyCloudyDay;
    dayList["cloudy"] = Overcast;
    //dayList["low level cloud"] = NotAvailable;
    //dayList["medium level cloud"] = NotAvailable;
    //dayList["sandstorm"] = NotAvailable;
    dayList["drizzle"] = LightRain;
    dayList["misty"] = Mist;
    dayList["mist"] = Mist;
    dayList["fog"] = Mist;
    dayList["foggy"] = Mist;
    dayList["tropical storm"] = Thunderstorm;
    dayList["hazy"] = NotAvailable;
    dayList["light shower"] = Showers;
    dayList["light rain shower"] = Showers;
    dayList["light showers"] = Showers;
    dayList["light rain"] = Showers;
    dayList["heavy rain"] = Rain;
    dayList["heavy showers"] = Rain;
    dayList["heavy shower"] = Rain;
    dayList["thundery shower"] = Thunderstorm;
    dayList["thunderstorm"] = Thunderstorm;
    dayList["cloudy with sleet"] = RainSnow;
    dayList["sleet shower"] = RainSnow;
    dayList["sleet showers"] = RainSnow;
    dayList["sleet"] = RainSnow;
    dayList["cloudy with hail"] = Hail;
    dayList["hail shower"] = Hail;
    dayList["hail showers"] = Hail;
    dayList["hail"] = Hail;
    dayList["light snow"] = LightSnow;
    dayList["light snow shower"] = Flurries;
    dayList["light snow showers"] = Flurries;
    dayList["cloudy with light snow"] = LightSnow;
    dayList["heavy snow"] = Snow;
    dayList["heavy snow shower"] = Snow;
    dayList["heavy snow showers"] = Snow;
    dayList["cloudy with heavy snow"] = Snow;
    dayList["na"] = NotAvailable;
    return dayList;
}

QMap<QString, IonInterface::ConditionIcons> UKMETIon::setupNightIconMappings(void)
{
    QMap<QString, ConditionIcons> nightList;
    nightList["clear"] = ClearNight;
    nightList["clear intervals"] = PartlyCloudyNight;
    nightList["sunny intervals"] = PartlyCloudyDay; // it's not really sunny
    nightList["sunny"] = ClearDay;
    nightList["cloudy"] = Overcast;
    nightList["partly cloudy"] = PartlyCloudyNight;
    nightList["drizzle"] = LightRain;
    nightList["misty"] = Mist;
    nightList["mist"] = Mist;
    nightList["fog"] = Mist;
    nightList["foggy"] = Mist;
    nightList["tropical storm"] = Thunderstorm;
    nightList["hazy"] = NotAvailable;
    nightList["light shower"] = Showers;
    nightList["light rain shower"] = Showers;
    nightList["light showers"] = Showers;
    nightList["light rain"] = Showers;
    nightList["heavy rain"] = Rain;
    nightList["heavy showers"] = Rain;
    nightList["heavy shower"] = Rain;
    nightList["thundery shower"] = Thunderstorm;
    nightList["thunderstorm"] = Thunderstorm;
    nightList["cloudy with sleet"] = NotAvailable;
    nightList["sleet shower"] = NotAvailable;
    nightList["sleet showers"] = NotAvailable;
    nightList["sleet"] = NotAvailable;
    nightList["cloudy with hail"] = Hail;
    nightList["hail shower"] = Hail;
    nightList["hail showers"] = Hail;
    nightList["hail"] = Hail;
    nightList["light snow"] = LightSnow;
    nightList["light snow shower"] = Flurries;
    nightList["light snow showers"] = Flurries;
    nightList["cloudy with light snow"] = LightSnow;
    nightList["heavy snow"] = Snow;
    nightList["heavy snow shower"] = Snow;
    nightList["heavy snow showers"] = Snow;
    nightList["cloudy with heavy snow"] = Snow;
    nightList["na"] = NotAvailable;

    return nightList;
}

QMap<QString, IonInterface::ConditionIcons> const& UKMETIon::dayIcons(void)
{
    static QMap<QString, ConditionIcons> const dval = setupDayIconMappings();
    return dval;
}

QMap<QString, IonInterface::ConditionIcons> const& UKMETIon::nightIcons(void)
{
    static QMap<QString, ConditionIcons> const nval = setupNightIconMappings();
    return nval;
}

// Get a specific Ion's data
bool UKMETIon::updateIonSource(const QString& source)
{
    // We expect the applet to send the source in the following tokenization:
    // ionname|validate|place_name - Triggers validation of place
    // ionname|weather|place_name - Triggers receiving weather of place

    QStringList sourceAction = source.split('|');

    // Guard: if the size of array is not 3 then we have bad data, return an error
    if (sourceAction.size() < 3) {
        setData(source, "validate", "bbcukmet|malformed");
        return true;
    }

    if (sourceAction[1] == "validate" && sourceAction.size() >= 3) {
        // Look for places to match
        findPlace(sourceAction[2], source);
        return true;
    } else if (sourceAction[1] == "weather" && sourceAction.size() >= 3) {
        if (sourceAction.count() >= 3) {
            if (sourceAction[2].isEmpty()) {
                setData(source, "validate", "bbcukmet|malformed");
                return true;
            }
            d->m_place[QString("bbcukmet|%1").arg(sourceAction[2])].XMLurl = sourceAction[3];
            getXMLData(QString("%1|%2").arg(sourceAction[0]).arg(sourceAction[2]));
            return true;
        } else {
            return false;
        }
    } else {
        setData(source, "validate", "bbcukmet|malformed");
        return true;
    }

    return false;
}

// Gets specific city XML data
void UKMETIon::getXMLData(const QString& source)
{
    KUrl url;
    url = d->m_place[source].XMLurl;

    d->m_job = KIO::get(url.url(), KIO::Reload, KIO::HideProgressInfo);
    d->m_job->addMetaData("cookies", "none"); // Disable displaying cookies
    d->m_obsJobXml.insert(d->m_job, new QXmlStreamReader);
    d->m_obsJobList.insert(d->m_job, source);

    if (d->m_job) {
        connect(d->m_job, SIGNAL(data(KIO::Job *, const QByteArray &)), this,
                SLOT(observation_slotDataArrived(KIO::Job *, const QByteArray &)));
        connect(d->m_job, SIGNAL(result(KJob *)), this, SLOT(observation_slotJobFinished(KJob *)));
    }
}

// Parses city list and gets the correct city based on ID number
void UKMETIon::findPlace(const QString& place, const QString& source)
{
    KUrl url;
    url = "http://www.bbc.co.uk/cgi-perl/weather/search/new_search.pl?x=0&y=0&=Submit&search_query=" + place + "&tmpl=wap";

    d->m_job = KIO::get(url.url(), KIO::Reload, KIO::HideProgressInfo);
    d->m_job->addMetaData("cookies", "none"); // Disable displaying cookies
    d->m_jobXml.insert(d->m_job, new QXmlStreamReader);
    d->m_jobList.insert(d->m_job, source);

    if (d->m_job) {
        connect(d->m_job, SIGNAL(data(KIO::Job *, const QByteArray &)), this,
                SLOT(setup_slotDataArrived(KIO::Job *, const QByteArray &)));
        connect(d->m_job, SIGNAL(result(KJob *)), this, SLOT(setup_slotJobFinished(KJob *)));

        // Handle redirects for direct hit places.
        connect(d->m_job, SIGNAL(redirection(KIO::Job *, const KUrl &)), this,
                SLOT(setup_slotRedirected(KIO::Job *, const KUrl &)));
    }
}

void UKMETIon::getFiveDayForecast(const QString& source)
{
    KUrl url;
    url = d->m_place[source].XMLforecastURL;
    QString xmlMap = d->m_place[source].XMLforecastURL;
    xmlMap.replace("weather/5day.shtml", "weather/mobile/5day.wml");
    url = xmlMap;

    d->m_job = KIO::get(url.url(), KIO::Reload, KIO::HideProgressInfo);
    d->m_job->addMetaData("cookies", "none"); // Disable displaying cookies
    d->m_forecastJobXml.insert(d->m_job, new QXmlStreamReader);
    d->m_forecastJobList.insert(d->m_job, source);

    if (d->m_job) {
        connect(d->m_job, SIGNAL(data(KIO::Job *, const QByteArray &)), this,
                SLOT(forecast_slotDataArrived(KIO::Job *, const QByteArray &)));
        connect(d->m_job, SIGNAL(result(KJob *)), this, SLOT(forecast_slotJobFinished(KJob *)));
    }
}

bool UKMETIon::readSearchXMLData(const QString& key, QXmlStreamReader& xml)
{

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "wml") {
                parseSearchLocations(key, xml);
            } else {
                parseUnknownElement(xml);
            }
        }
    }

    return !xml.error();
}

void UKMETIon::parseSearchLocations(const QString& source, QXmlStreamReader& xml)
{
    int flag = 0;
    QString url;
    QString place;
    QStringList tokens;
    QString tmp;
    int counter = 2;
    int currentParagraph = 0;

    Q_ASSERT(xml.isStartElement() && xml.name() == "wml");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "wml") {
            break;
        }

        if (xml.isStartElement() && xml.name() == "p") {
            currentParagraph++;
        }

        if (currentParagraph == 2) {
            if (xml.isCharacters() && !xml.isWhitespace())  {
                QString dataText = xml.text().toString().trimmed();
                if (dataText.contains("No locations")) {
                    break;
                }
            }
        }

        if (xml.isStartElement()) {
            if (xml.name() == "a" && !xml.attributes().value("href").isEmpty()) {
                if (xml.attributes().value("href").toString().contains("5day.wml")) {

                    // Split URL to determine station ID number
                    tokens = xml.attributes().value("href").toString().split('=');
                    if (xml.attributes().value("href").toString().contains("world")) {
                        url = "http://feeds.bbc.co.uk/weather/feeds/obs/world/" + tokens[1] + ".xml";
                        flag = 0;
                    } else {
                        url = "http://feeds.bbc.co.uk/weather/feeds/obs/id/" + tokens[1] + ".xml";
                        flag = 1;
                    }
                    place = xml.readElementText();
                    tmp = QString("bbcukmet|%1").arg(place);

                    // Duplicate places can exist
                    if (d->m_locations.contains(tmp)) {

                        QString dupePlace = place;
                        tmp = QString("bbcukmet|%1").arg(QString("%1 (#%2)").arg(dupePlace).arg(counter));
                        place = QString("%1 (#%2)").arg(dupePlace).arg(counter);
                        counter++;
                    }

                    if (flag) {  // This is a UK specific location
                        d->m_place[tmp].XMLurl = url;
                        d->m_place[tmp].place = place;
                        d->m_place[tmp].ukPlace = true;
                    } else {
                        d->m_place[tmp].XMLurl = url;
                        d->m_place[tmp].place = place;
                        d->m_place[tmp].ukPlace = false;
                    }
                    d->m_locations.append(tmp);
                }
            }
        }
    }
    validate(source);
}

// handle when no XML tag is found
void UKMETIon::parseUnknownElement(QXmlStreamReader& xml)
{
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            parseUnknownElement(xml);
        }
    }
}

void UKMETIon::setup_slotRedirected(KIO::Job *job, const KUrl &url)
{
    QString obsUrl;
    QString place;
    QString tmp;
    bool flag = false;
    QStringList tokens = url.url().split('=');
    if (url.url().contains("xhtml")) { // We don't care about the first redirection (there is two)
        if (url.url().contains("world")) {
            obsUrl = "http://feeds.bbc.co.uk/weather/feeds/obs/world/" + tokens[2] + ".xml";
            flag = false;
        } else {
            obsUrl = "http://feeds.bbc.co.uk/weather/feeds/obs/id/" + tokens[2] + ".xml";
            flag = true;
        }
        place = d->m_jobList[job].split('|')[2]; // Contains the source name (place in this case)
        tmp = QString("bbcukmet|%1").arg(place);
        place[0] = place[0].toUpper();

        if (flag) { // This is a UK specific location
            d->m_place[tmp].XMLurl = obsUrl;
            d->m_place[tmp].place = place;
            d->m_place[tmp].ukPlace = true;
        } else {
            d->m_place[tmp].XMLurl = obsUrl;
            d->m_place[tmp].place = place;
            d->m_place[tmp].ukPlace = false;
        }
        d->m_locations.append(tmp);
        validate(d->m_jobList[job]);
    }
}

void UKMETIon::setup_slotDataArrived(KIO::Job *job, const QByteArray &data)
{
    QByteArray local = data;
    if (data.isEmpty() || !d->m_jobXml.contains(job)) {
        return;
    }

    // XXX: BBC doesn't convert unicode strings so this breaks XML formatting. Not pretty.
    if (local.startsWith("<?xml version")) {
        local.replace("<?xml version=\"1.0\"?>", "<?xml version=\"1.0\" encoding=\"cp1252\" ?>");
    }

    // Send to xml.
    d->m_jobXml[job]->addData(local);
}

void UKMETIon::setup_slotJobFinished(KJob *job)
{
    if (job->error() == 149) {
        setData(d->m_jobList[job], "validate", QString("bbcukmet|timeout"));
        disconnectSource(d->m_jobList[job], this);
        d->m_jobList.remove(job);
        delete d->m_jobXml[job];
        d->m_jobXml.remove(job);
        return;
    }
    // If Redirected, don't go to this routine
    if (!d->m_locations.contains(QString("bbcukmet|%1").arg(d->m_jobList[job]))) {
        QXmlStreamReader *reader = d->m_jobXml.value(job);
        if (reader) {
            readSearchXMLData(d->m_jobList[job], *reader);
        }
    }
    d->m_jobList.remove(job);
    delete d->m_jobXml[job];
    d->m_jobXml.remove(job);
}

void UKMETIon::observation_slotDataArrived(KIO::Job *job, const QByteArray &data)
{
    QByteArray local = data;
    if (data.isEmpty() || !d->m_obsJobXml.contains(job)) {
        return;
    }

    // XXX: I don't know what to say about this. But horrible. We can't really do much about this :/
    // No, it's not UTF-8, it really lies.
    if (local.startsWith("<?xml version")) {
        local.replace("encoding=\"UTF-8\"?>", "encoding=\"cp1252\" ?>");
    }

    // Send to xml.
    d->m_obsJobXml[job]->addData(local);
}

void UKMETIon::observation_slotJobFinished(KJob *job)
{
    setData(d->m_obsJobList[job], Data());

    QXmlStreamReader *reader = d->m_obsJobXml.value(job);
    if (reader) {
        readObservationXMLData(d->m_obsJobList[job], *reader);
    }

    d->m_obsJobList.remove(job);
    delete d->m_obsJobXml[job];
    d->m_obsJobXml.remove(job);
}

void UKMETIon::forecast_slotDataArrived(KIO::Job *job, const QByteArray &data)
{
    QByteArray local = data;
    if (data.isEmpty() || !d->m_forecastJobXml.contains(job)) {
        return;
    }

    // XXX: BBC doesn't convert unicode strings so this breaks XML formatting. Not pretty.
    // No, it's not UTF-8, it really lies.
    if (local.startsWith("<?xml version")) {
        local.replace("<?xml version=\"1.0\"?>", "<?xml version=\"1.0\" encoding=\"cp1252\" ?>");
    }
    // Send to xml.
    d->m_forecastJobXml[job]->addData(local);
}

void UKMETIon::forecast_slotJobFinished(KJob *job)
{
    setData(d->m_forecastJobList[job], Data());
    QXmlStreamReader *reader = d->m_forecastJobXml.value(job);
    if (reader) {
        readFiveDayForecastXMLData(d->m_forecastJobList[job], *reader);
    }

    d->m_forecastJobList.remove(job);
    delete d->m_forecastJobXml[job];
    d->m_forecastJobXml.remove(job);
}

void UKMETIon::parsePlaceObservation(const QString &source, WeatherData& data, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "rss");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "rss") {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "channel") {
                parseWeatherChannel(source, data, xml);
            }
        }
    }
}

void UKMETIon::parseWeatherChannel(const QString& source, WeatherData& data, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "channel");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "channel") {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "title") {
                data.stationName = xml.readElementText().split("Observations for")[1].trimmed();

                data.stationName.replace("United Kingdom", i18n("UK"));
                data.stationName.replace("United States of America", i18n("USA"));

            } else if (xml.name() == "item") {
                parseWeatherObservation(source, data, xml);
            } else {
                parseUnknownElement(xml);
            }
        }
    }
}

void UKMETIon::parseWeatherObservation(const QString& source, WeatherData& data, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "item");

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "item") {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "title") {
                QString conditionString = xml.readElementText();

                // Get the observation time.
                QStringList conditionData = conditionString.split(':');

                data.obsTime = conditionData[0];
                // Friday at 0200 GMT
                d->m_dateFormat =  QDateTime::fromString(data.obsTime.split("at")[1].trimmed(), "hhmm 'GMT'");
                data.iconPeriodHour = d->m_dateFormat.toString("hh").toInt();
                data.iconPeriodMinute = d->m_dateFormat.toString("mm").toInt();
                //data.iconPeriodAP = d->m_dateFormat.toString("ap");

                data.condition = conditionData[1].split('.')[0].trimmed();

            } else if (xml.name() == "link") {
                d->m_place[source].XMLforecastURL = xml.readElementText();
            } else if (xml.name() == "description") {
                QString observeString = xml.readElementText();
                QStringList observeData = observeString.split(':');

                data.temperature_C = observeData[1].split(QChar(176))[0].trimmed();
                data.temperature_F = observeData[1].split('(')[1].split(QChar(176))[0];

                data.windDirection = observeData[2].split(',')[0].trimmed();
                data.windSpeed_miles = observeData[3].split(',')[0].split(' ')[1];

                data.humidity = observeData[4].split(',')[0].split(' ')[1];

                data.pressure = observeData[5].split(',')[0].split(' ')[1].split("mB")[0];

                data.pressureTendency = observeData[5].split(',')[1].trimmed();

                data.visibilityStr = observeData[6].trimmed();
            } else if (xml.name() == "lat") {
                const QString ordinate = xml.readElementText();
                data.latitude = ordinate.toDouble();
            } else if (xml.name() == "long") {
                const QString ordinate = xml.readElementText();
                data.longitude = ordinate.toDouble();
            } else {
                parseUnknownElement(xml);
            }
        }
    }
}

bool UKMETIon::readObservationXMLData(const QString& source, QXmlStreamReader& xml)
{
    WeatherData data;
    bool haveObservation = false;
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "rss") {
                parsePlaceObservation(source, data, xml);
                haveObservation = true;
            } else {
                parseUnknownElement(xml);
            }
        }

    }

    if (!haveObservation) {
        return false;
    }
    d->m_weatherData[source] = data;

    // Get the 5 day forecast info next.
    getFiveDayForecast(source);

    return !xml.error();
}

bool UKMETIon::readFiveDayForecastXMLData(const QString& source, QXmlStreamReader& xml)
{
    bool haveFiveDay = false;
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement()) {
            break;
        }

        if (xml.isStartElement()) {
            if (xml.name() == "wml") {
                parseFiveDayForecast(source, xml);
                haveFiveDay = true;
            } else {
                parseUnknownElement(xml);
            }
        }
    }
    if (!haveFiveDay) return false;
    updateWeather(source);
    return !xml.error();
}

void UKMETIon::parseFiveDayForecast(const QString& source, QXmlStreamReader& xml)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "wml");
    bool validNumber = false;
    int currentParagraph = 0;
    bool skipPlace = false;
    int dataItem = 0;

    enum DataItem {
        Day,
        Summary,
        MaxTemp,
        MinTemp,
        WindSpeed
    };

    // Flush out the old forecasts when updating.
    d->m_weatherData[source].forecasts.clear();

    WeatherData::ForecastInfo *forecast = new WeatherData::ForecastInfo;

    QRegExp numParser("(Max|Min|Wind)\\s+-*([0-9]+)");
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == "p") {
            currentParagraph++;
        }

        if (currentParagraph == 3) {
            if (xml.isCharacters() && !xml.isWhitespace())  {
                QString dataText = xml.text().toString().trimmed();
                if (!skipPlace) {
                    skipPlace = true;
                } else {
                    if (numParser.indexIn(dataText) != -1 && numParser.capturedTexts().count() >= 3) {
                        validNumber = true;
                    }
                    switch (dataItem) {
                    case Day:
                        forecast->period = dataText;
                        dataItem++;
                        break;
                    case Summary:
                        forecast->summary = dataText;
                        forecast->iconName = getWeatherIcon(dayIcons(), forecast->summary.toLower());
                        dataItem++;
                        break;
                    case MaxTemp:
                        forecast->tempHigh = numParser.capturedTexts()[0].remove("Max").toInt();
                        dataItem++;
                        validNumber = false;
                        break;
                    case MinTemp:
                        forecast->tempLow = numParser.capturedTexts()[0].remove("Min").toInt();
                        dataItem++;
                        validNumber = false;
                        break;
                    case WindSpeed:
                        forecast->windSpeed = numParser.capturedTexts()[0].remove("Wind").toInt();
                        forecast->windDirection = dataText.split('(')[1].split(')')[0];
                        dataItem = 0;
                        d->m_weatherData[source].forecasts.append(forecast);
                        forecast = new WeatherData::ForecastInfo;
                        validNumber = false;
                        break;
                    };
                }
            }
        }
    }

    delete forecast;
}

void UKMETIon::validate(const QString& source)
{
    bool beginflag = true;

    if (!d->m_locations.count()) {
        QStringList invalidPlace = source.split('|');
        if (d->m_place[QString("bbcukmet|%1").arg(invalidPlace[2])].place.isEmpty()) {
            setData(source, "validate", QString("bbcukmet|invalid|multiple|%1").arg(invalidPlace[2]));
        }
        d->m_locations.clear();
        return;
    } else {
        QString placeList;
        foreach(const QString &place, d->m_locations) {
            if (beginflag) {
                placeList.append(QString("%1|extra|%2").arg(place.split('|')[1]).arg(d->m_place[place].XMLurl));
                beginflag = false;
            } else {
                placeList.append(QString("|place|%1|extra|%2").arg(place.split('|')[1]).arg(d->m_place[place].XMLurl));
            }
        }
        if (d->m_locations.count() > 1) {
            setData(source, "validate", QString("bbcukmet|valid|multiple|place|%1").arg(placeList));
        } else {
            placeList[0] = placeList[0].toUpper();
            setData(source, "validate", QString("bbcukmet|valid|single|place|%1").arg(placeList));
        }
    }
    d->m_locations.clear();
}

void UKMETIon::updateWeather(const QString& source)
{
    QString weatherSource = source;
    weatherSource.replace("bbcukmet|", "bbcukmet|weather|");
    weatherSource.append(QString("|%1").arg(d->m_place[source].XMLurl));

    QMap<QString, QString> dataFields;
    QStringList fieldList;
    QVector<QString> forecastList;
    int i = 0;

    Plasma::DataEngine::Data data;

    data.insert("Place", place(source));
    data.insert("Station", station(source));
    data.insert("Observation Period", observationTime(source));
    data.insert("Current Conditions", condition(source));

    const double observationSeconds = 60.0 * (periodMinute(source) + 60.0 * periodHour(source));

    const double lati = periodLatitude(source);
    const double longi = periodLongitude(source);
    const QDate today = QDate::currentDate();
    const double sunrise = calculateSunriseTime(today.day(), today.month(), today.year(), lati, longi);
    const double sunset = calculateSunsetTime(today.day(), today.month(), today.year(), lati, longi);

    //kDebug() << "Calculated sunrise and sunset as " << sunrise << ", " << sunset;
    //kDebug() << "Observation was made at " << observationSeconds;

    // Tell applet which icon to use for conditions and provide mapping for condition type to the icons to display
    if (observationSeconds >= 0 && observationSeconds < sunrise) {
        //kDebug() << "Before sunrise - using night icons\n";
        data.insert("Condition Icon", getWeatherIcon(nightIcons(), condition(source)));
    } else if (observationSeconds >= sunset) {
        //kDebug() << "After sunset - using night icons\n";
        data.insert("Condition Icon", getWeatherIcon(nightIcons(), condition(source)));
    } else {
        //kDebug() << "Using daytime icons\n";
        data.insert("Condition Icon", getWeatherIcon(dayIcons(), condition(source)));
    }

    data.insert("Humidity", humidity(source));
    data.insert("Visibility", visibility(source));

    dataFields = temperature(source);
    data.insert("Temperature", dataFields["temperature"]);
    data.insert("Temperature Unit", dataFields["temperatureUnit"]);

    dataFields = pressure(source);
    data.insert("Pressure", dataFields["pressure"]);
    data.insert("Pressure Unit", dataFields["pressureUnit"]);
    data.insert("Pressure Tendency", dataFields["pressureTendency"]);

    dataFields = wind(source);
    data.insert("Wind Speed", dataFields["windSpeed"]);
    data.insert("Wind Speed Unit", dataFields["windUnit"]);
    data.insert("Wind Direction", dataFields["windDirection"]);

    // 5 Day forecast info
    forecastList = forecasts(source);

    // Set number of forecasts per day/night supported
    data.insert("Total Weather Days", d->m_weatherData[source].forecasts.size());

    foreach(const QString &forecastItem, forecastList) {
        fieldList = forecastItem.split('|');

        data.insert(QString("Short Forecast Day %1").arg(i), QString("%1|%2|%3|%4|%5|%6") \
                .arg(fieldList[0]).arg(fieldList[1]).arg(fieldList[2]).arg(fieldList[3]) \
                .arg(fieldList[4]).arg(fieldList[5]));
        i++;
    }

    data.insert("Credit", i18n("Supported by backstage.bbc.co.uk / Data from UK MET Office"));
    data.insert("Credit Url", d->m_place[source].XMLforecastURL);
    
    setData(weatherSource, data);
}

QString UKMETIon::place(const QString& source)
{
    return d->m_weatherData[source].stationName;
}

QString UKMETIon::station(const QString& source)
{
    return d->m_weatherData[source].stationName;
}

QString UKMETIon::observationTime(const QString& source)
{
    return d->m_weatherData[source].obsTime;
}

/*
bool UKMETIon::night(const QString& source)
{
    if (d->m_weatherData[source].iconPeriodAP == "pm") {
        return true;
    }
    return false;
} */

int UKMETIon::periodHour(const QString& source)
{
    return d->m_weatherData[source].iconPeriodHour;
}

int UKMETIon::periodMinute(const QString& source)
{
    return d->m_weatherData[source].iconPeriodMinute;
}

double UKMETIon::periodLatitude(const QString& source)
{
    return d->m_weatherData[source].latitude;
}

double UKMETIon::periodLongitude(const QString& source)
{
    return d->m_weatherData[source].longitude;
}

QString UKMETIon::condition(const QString& source)
{
    return d->m_weatherData[source].condition;
}

QMap<QString, QString> UKMETIon::temperature(const QString& source)
{
    QMap<QString, QString> temperatureInfo;

    temperatureInfo.insert("temperature", QString(d->m_weatherData[source].temperature_C));
    temperatureInfo.insert("temperatureUnit", QString::number(WeatherUtils::Celsius));
    return temperatureInfo;
}

QMap<QString, QString> UKMETIon::wind(const QString& source)
{
    QMap<QString, QString> windInfo;
    if (d->m_weatherData[source].windSpeed_miles == "N/A") {
        windInfo.insert("windSpeed", "N/A");
        windInfo.insert("windUnit", QString::number(WeatherUtils::NoUnit));
    } else {
        windInfo.insert("windSpeed", QString(d->m_weatherData[source].windSpeed_miles));
        windInfo.insert("windUnit", QString::number(WeatherUtils::MilesPerHour));
    }
    windInfo.insert("windDirection", d->m_weatherData[source].windDirection);
    return windInfo;
}

QString UKMETIon::humidity(const QString& source)
{
    if (d->m_weatherData[source].humidity == "N/A%") {
        return "N/A";
    }
    return d->m_weatherData[source].humidity;
}

QString UKMETIon::visibility(const QString& source)
{
    return d->m_weatherData[source].visibilityStr;
}

QMap<QString, QString> UKMETIon::pressure(const QString& source)
{
    QMap<QString, QString> pressureInfo;
    if (d->m_weatherData[source].pressure == "N/A") {
        pressureInfo.insert("pressure", "N/A");
        return pressureInfo;
    }

    pressureInfo.insert("pressure", QString(d->m_weatherData[source].pressure));
    pressureInfo.insert("pressureUnit", QString::number(WeatherUtils::Millibars));

    pressureInfo.insert("pressureTendency", d->m_weatherData[source].pressureTendency);
    return pressureInfo;
}

QVector<QString> UKMETIon::forecasts(const QString& source)
{
    QVector<QString> forecastData;

    for (int i = 0; i < d->m_weatherData[source].forecasts.size(); ++i) {

        if (d->m_weatherData[source].forecasts[i]->period.contains("Saturday")) {
            d->m_weatherData[source].forecasts[i]->period.replace("Saturday", i18n("Sat"));
        }

        if (d->m_weatherData[source].forecasts[i]->period.contains("Sunday")) {
            d->m_weatherData[source].forecasts[i]->period.replace("Sunday", i18n("Sun"));
        }

        if (d->m_weatherData[source].forecasts[i]->period.contains("Monday")) {
            d->m_weatherData[source].forecasts[i]->period.replace("Monday", i18n("Mon"));
        }

        if (d->m_weatherData[source].forecasts[i]->period.contains("Tuesday")) {
            d->m_weatherData[source].forecasts[i]->period.replace("Tuesday", i18n("Tue"));
        }

        if (d->m_weatherData[source].forecasts[i]->period.contains("Wednesday")) {
            d->m_weatherData[source].forecasts[i]->period.replace("Wednesday", i18n("Wed"));
        }

        if (d->m_weatherData[source].forecasts[i]->period.contains("Thursday")) {
            d->m_weatherData[source].forecasts[i]->period.replace("Thursday", i18n("Thu"));
        }
        if (d->m_weatherData[source].forecasts[i]->period.contains("Friday")) {
            d->m_weatherData[source].forecasts[i]->period.replace("Friday", i18n("Fri"));
        }

        forecastData.append(QString("%1|%2|%3|%4|%5|%6") \
                            .arg(d->m_weatherData[source].forecasts[i]->period) \
                            .arg(d->m_weatherData[source].forecasts[i]->iconName) \
                            .arg(d->m_weatherData[source].forecasts[i]->summary) \
                            .arg(d->m_weatherData[source].forecasts[i]->tempHigh) \
                            .arg(d->m_weatherData[source].forecasts[i]->tempLow) \
                            .arg("N/U"));
        //.arg(d->m_weatherData[source].forecasts[i]->windSpeed)
        //arg(d->m_weatherData[source].forecasts[i]->windDirection));
    }

    return forecastData;
}

#include "ion_bbcukmet.moc"
