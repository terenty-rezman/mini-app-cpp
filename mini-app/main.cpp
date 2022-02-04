#include "miniapp.h"
#include <QtWidgets/QApplication>
#include <qfile.h>

QString read_style(const QString& file_name)
{
	QFile file(file_name);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return file.readAll();
	}
	return {};
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    miniapp w;
	w.setStyleSheet(read_style("stylesheet.qss"));
    w.show();

    return a.exec();
}
