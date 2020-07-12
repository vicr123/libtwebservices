/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "twmeteorology.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

struct twMeteorologyPrivate {
    QDateTime dateTime;
    double lat;
    double lon;

    QTime sunrise;
    QTime sunset;

    QList<QNetworkReply*> pendingReplies;
    QNetworkAccessManager mgr;
};

twMeteorology::twMeteorology(QObject* parent) : QObject(parent) {
    d = new twMeteorologyPrivate();
    d->dateTime = QDateTime::currentDateTime();
}

twMeteorology::~twMeteorology() {
    delete d;
}

void twMeteorology::setLocation(double lat, double lon) {
    d->lat = lat;
    d->lon = lon;

    updateData();
}

void twMeteorology::setDateTime(QDateTime dateTime) {
    d->dateTime = dateTime;

    updateData();
}

QTime twMeteorology::sunrise() const {
    return d->sunrise;
}

QTime twMeteorology::sunset() const {
    return d->sunset;
}

void twMeteorology::updateData() {
    for (QNetworkReply* reply : d->pendingReplies) {
        reply->abort();
    }

    //Query the met.no API for details
    QString utcOffset = QStringLiteral("+");
    int offsetFromUtc = d->dateTime.offsetFromUtc();
    if (offsetFromUtc < 0) {
        utcOffset += QStringLiteral("-");
        offsetFromUtc *= -1;
    }
    utcOffset.append(QTime::fromMSecsSinceStartOfDay(offsetFromUtc * 1000).toString("HH:mm"));

    QNetworkRequest sunriseSunsetReq(QUrl(QStringLiteral("https://api.met.no/weatherapi/sunrise/2.0/.json?lat=%1&lon=%2&date=%3&offset=%4")
            .arg(d->lat)
            .arg(d->lon)
            .arg(d->dateTime.toString("yyyy-MM-dd"))
            .arg(utcOffset)));
    QNetworkReply* sunriseSunsetRep = d->mgr.get(sunriseSunsetReq);
    connect(sunriseSunsetRep, &QNetworkReply::finished, this, [ = ] {
        d->pendingReplies.removeOne(sunriseSunsetRep);

        if (sunriseSunsetRep->error() == QNetworkReply::NoError) {
            QByteArray data = sunriseSunsetRep->readAll();

            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonArray dates = doc.object().value("location").toObject().value("time").toArray();
            if (!dates.isEmpty()) {
                QJsonObject timeData = dates.first().toObject();
                if (timeData.contains("sunrise")) {
                    d->sunrise = QDateTime::fromString(timeData.value("sunrise").toObject().value("time").toString().left(19), "yyyy-MM-ddThh:mm:ss").time();
                }
                if (timeData.contains("sunset")) {
                    d->sunset = QDateTime::fromString(timeData.value("sunset").toObject().value("time").toString().left(19), "yyyy-MM-ddThh:mm:ss").time();
                }

                emit sunriseSunsetChanged();
            }
        }

        sunriseSunsetRep->deleteLater();
    });
    d->pendingReplies.append(sunriseSunsetRep);
}
