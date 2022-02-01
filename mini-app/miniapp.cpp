#include "miniapp.h"
#include "qmath.h"
#include <QDoubleValidator>
#include <qmessagebox.h>
#include <qsettings.h>

namespace
{
	std::pair<double, double> calc_q(double Dp, double Ds, double A, double freq)
	{
		// Основные вычисления:
		// Перевод в систему СИ:
		double A_meters = A / 1000;     // амплитуда: мм -> м
		double Freq = 2 * M_PI * freq;  // частота:   Гц -> рад/c

		double Dp_meters = Dp / 1000;   // площадь поршня:   мм -> м
		double Ds_meters = Ds / 1000;   // площадь штока:    мм -> м  

		// Площадь рабочей поверхности:
		double S_work_area = (M_PI / 4) * (Dp_meters * Dp_meters - Ds_meters * Ds_meters); // в м^2

		// Основная формула:
		double Q_m3s = Freq * A_meters * S_work_area; // расход в ед. СИ
		double Q_litrmin = Q_m3s * (1000.0 * 60.0); // расход в л/мин

		return { Q_m3s, Q_litrmin };
	}
}

miniapp::miniapp(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	this->setWindowTitle("Вычисление расхода");

	auto [
		Dp_widget,
		Ds_widget,
		A_widget,
		freq_widget,
		result_Qmin,
		result_Qsek
	] = find_widgets();

	QPushButton* calc_button = this->findChild<QPushButton*>("calc");

	Dp_widget->setValidator(new QDoubleValidator());
	Ds_widget->setValidator(new QDoubleValidator());
	A_widget->setValidator(new QDoubleValidator());
	freq_widget->setValidator(new QDoubleValidator());

	QObject::connect(calc_button, &QPushButton::clicked, this, &miniapp::read_fields_and_calc_q);

	QObject::connect(Dp_widget, &QLineEdit::returnPressed, this, &miniapp::read_fields_and_calc_q);
	QObject::connect(Ds_widget, &QLineEdit::returnPressed, this, &miniapp::read_fields_and_calc_q);
	QObject::connect(A_widget, &QLineEdit::returnPressed, this, &miniapp::read_fields_and_calc_q);
	QObject::connect(freq_widget, &QLineEdit::returnPressed, this, &miniapp::read_fields_and_calc_q);

	load_settings();
}

void miniapp::closeEvent(QCloseEvent* event)
{
	QSettings settings = create_settings_obj();

	settings.beginGroup("mainwindow");
	settings.setValue("size", this->size());
	settings.setValue("pos", this->pos());
	settings.endGroup();

	auto [
		Dp_widget,
		Ds_widget,
		A_widget,
		freq_widget,
		result_Qmin,
		result_Qsek
	] = find_widgets();

	settings.beginGroup("fields");
	settings.setValue("Dp_widget", Dp_widget->text());
	settings.setValue("Ds_widget", Ds_widget->text());
	settings.setValue("A_widget", A_widget->text());
	settings.setValue("freq_widget", freq_widget->text());
	settings.setValue("result_Qmin", result_Qmin->text());
	settings.setValue("result_Qsek", result_Qsek->text());
	settings.endGroup();
}

std::tuple<QLineEdit*, QLineEdit*, QLineEdit*, QLineEdit*, QLineEdit*, QLineEdit*>
miniapp::find_widgets()
{
	QLineEdit* Dp_widget = this->findChild<QLineEdit*>("Dp");
	QLineEdit* Ds_widget = this->findChild<QLineEdit*>("Ds");
	QLineEdit* A_widget = this->findChild<QLineEdit*>("A");
	QLineEdit* freq_widget = this->findChild<QLineEdit*>("freq");

	QLineEdit* result_Qmin = this->findChild<QLineEdit*>("resultQmin");
	QLineEdit* result_Qsek = this->findChild<QLineEdit*>("resultQsek");

	return { Dp_widget, Ds_widget, A_widget, freq_widget, result_Qmin, result_Qsek };
}

void miniapp::read_fields_and_calc_q()
{
	auto [
		Dp_widget,
		Ds_widget,
		A_widget,
		freq_widget,
		result_Qmin,
		result_Qsek
	] = find_widgets();

	std::tuple<QString, const QLineEdit*, QString> id_and_parse_widget_and_error_msg[] = {
			{"Dp", Dp_widget, "Неверное значение поля Диаметр поршня."},
			{"Ds", Ds_widget, "Неверное значение поля Диаметр штока."},
			{"A", A_widget, "Неверное значение поля Амплитуда сигнала."},
			{"freq", freq_widget, "Неверное значение поля Частота сигнала."}
	};

	std::map<QString, double> args;

	for (auto [id, widget, error_msg] : id_and_parse_widget_and_error_msg) {
		bool ok = false;
		double arg = widget->text().toDouble(&ok);
		if (!ok) {
			QMessageBox::warning(this, "Неверное значение", error_msg);
			return;
		}
		args[id] = arg;
	}

	auto [Q_m3s, Q_litrmin] = calc_q(args["Dp"], args["Ds"], args["A"], args["freq"]);

	result_Qmin->setText(QString::number(Q_litrmin));
	result_Qsek->setText(QString::number(Q_m3s));
}

void miniapp::load_settings()
{
	QSettings settings = create_settings_obj();

	settings.beginGroup("mainwindow");
	resize(settings.value("size", QSize(650, 250)).toSize());
	move(settings.value("pos", QPoint(200, 200)).toPoint());
	settings.endGroup();

	auto [
		Dp_widget,
		Ds_widget,
		A_widget,
		freq_widget,
		result_Qmin,
		result_Qsek
	] = find_widgets();

	settings.beginGroup("fields");
	Dp_widget->setText(settings.value("Dp_widget", "").toString());
	Ds_widget->setText(settings.value("Ds_widget", "").toString());
	A_widget->setText(settings.value("A_widget", "").toString());
	freq_widget->setText(settings.value("freq_widget", "").toString());
	result_Qmin->setText(settings.value("result_Qmin", "").toString());
	result_Qsek->setText(settings.value("result_Qsek", "").toString());
	settings.endGroup();
}

QSettings miniapp::create_settings_obj()
{
	return QSettings("O", "q_calc");
}
