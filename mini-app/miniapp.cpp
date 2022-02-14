#include "miniapp.h"
#include "qmath.h"
#include <QDoubleValidator>
#include <qmessagebox.h>
#include <qsettings.h>
#include <qclipboard.h>
#include <qbuttongroup.h>
#include <qfile.h>

namespace
{
	std::tuple<double, double, double, double> calc_q_A(double Dp, double Ds, double A, double freq)
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
		double V = Freq * A_meters;
		double Q_m3s = V * S_work_area; // расход в ед. СИ
		double Q_litrmin = Q_m3s * (1000.0 * 60.0); // расход в л/мин

		return { Q_m3s, Q_litrmin, S_work_area, V * 1000.0 };
	}

	std::tuple<double, double, double> calc_q_Speed(double Dp, double Ds, double Speed)
	{
		// Основные вычисления:
		// Перевод в систему СИ:
		double Dp_meters = Dp / 1000;   // площадь поршня:   мм -> м
		double Ds_meters = Ds / 1000;   // площадь штока:    мм -> м  

		// Площадь рабочей поверхности:
		double S_work_area = (M_PI / 4) * (Dp_meters * Dp_meters - Ds_meters * Ds_meters); // в м^2

		// Основная формула:
		double V = Speed / 1000;
		double Q_m3s = V * S_work_area; // расход в ед. СИ
		double Q_litrmin = Q_m3s * (1000.0 * 60.0); // расход в л/мин

		return { Q_m3s, Q_litrmin, S_work_area };
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
		Speed_widget,
		freq_widget,
		result_Qmin,
		result_Qsek,
		result_S,
		Speed_checkbox
	] = find_widgets();

	QPushButton* calc_button = this->findChild<QPushButton*>("calc");

    QDoubleValidator* validator = new QDoubleValidator();
    validator->setLocale(QLocale::English);

    Dp_widget->setValidator(validator);
    Ds_widget->setValidator(validator);
    A_widget->setValidator(validator);
    freq_widget->setValidator(validator);
    Speed_widget->setValidator(validator);

	QObject::connect(calc_button, &QPushButton::clicked, this, &miniapp::read_fields_and_calc_q);

	QObject::connect(Dp_widget, &QLineEdit::returnPressed, this, &miniapp::read_fields_and_calc_q);
	QObject::connect(Ds_widget, &QLineEdit::returnPressed, this, &miniapp::read_fields_and_calc_q);
	QObject::connect(A_widget, &QLineEdit::returnPressed, this, &miniapp::read_fields_and_calc_q);
	QObject::connect(freq_widget, &QLineEdit::returnPressed, this, &miniapp::read_fields_and_calc_q);

	QPushButton* copy_qmin_button = this->findChild<QPushButton*>("copyQmin");
	QPushButton* copy_qsek_button = this->findChild<QPushButton*>("copyQsek");

	QClipboard* clipboard = QApplication::clipboard();

	QObject::connect(copy_qmin_button, &QPushButton::clicked, [=, result_Qmin = result_Qmin]() {
		if(!copy_qmin_button->text().isEmpty())
			clipboard->setText(result_Qmin->text());
	});

	QObject::connect(copy_qsek_button, &QPushButton::clicked, [=, result_Qsek = result_Qsek]() {
		if (!copy_qsek_button->text().isEmpty())
			clipboard->setText(result_Qsek->text());
	});

	QButtonGroup* exclusive_buttons = new QButtonGroup();

	QObject::connect(Speed_checkbox, &QCheckBox::stateChanged, this, &miniapp::enable_speed);

	enable_speed(false);

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
		Speed_widget,
		freq_widget,
		result_Qmin,
		result_Qsek,
		result_S,
		Speed_checkbox
	] = find_widgets();

	settings.beginGroup("fields");
	settings.setValue("Dp_widget", Dp_widget->text());
	settings.setValue("Ds_widget", Ds_widget->text());
	settings.setValue("A_widget", A_widget->text());
	settings.setValue("Speed_widget", Speed_widget->text());
	settings.setValue("freq_widget", freq_widget->text());
	settings.setValue("result_Qmin", result_Qmin->text());
	settings.setValue("result_Qsek", result_Qsek->text());
	settings.setValue("Speed_checkbox", Speed_checkbox->isChecked());
	settings.endGroup();
}

std::tuple<QLineEdit*, QLineEdit*, QLineEdit*, QLineEdit*, QLineEdit*, QLineEdit*, QLineEdit*, QLineEdit*, QCheckBox*>
miniapp::find_widgets()
{
	QLineEdit* Dp_widget = this->findChild<QLineEdit*>("Dp");
	QLineEdit* Ds_widget = this->findChild<QLineEdit*>("Ds");
	QLineEdit* A_widget = this->findChild<QLineEdit*>("A");
	QLineEdit* Speed_widget = this->findChild<QLineEdit*>("Speed");
	QLineEdit* freq_widget = this->findChild<QLineEdit*>("freq");

	QLineEdit* result_Qmin = this->findChild<QLineEdit*>("resultQmin");
	QLineEdit* result_Qsek = this->findChild<QLineEdit*>("resultQsek");
	QLineEdit* result_S = this->findChild<QLineEdit*>("resultS");

	QCheckBox* Speed_checkbox = this->findChild<QCheckBox*>("Speed_checkbox");

	return { Dp_widget, Ds_widget, A_widget, Speed_widget, freq_widget, result_Qmin, result_Qsek, result_S, Speed_checkbox };
}

void miniapp::read_fields_and_calc_q()
{
	auto [
		Dp_widget,
		Ds_widget,
		A_widget,
		Speed_widget,
		freq_widget,
		result_Qmin,
		result_Qsek,
		result_S,
		Speed_checkbox
	] = find_widgets();

	std::vector<std::tuple<QString, const QLineEdit*, QString>> id_and_widget_and_error = {
			{"Dp", Dp_widget, "Неверное значение поля Диаметр поршня."},
			{"Ds", Ds_widget, "Неверное значение поля Диаметр штока."},
	};

	if (Speed_checkbox->isChecked()) {
		id_and_widget_and_error.push_back({ "Speed", Speed_widget, "Неверное значение поля Скорость." });
	}
	else {
		id_and_widget_and_error.push_back({ "A", A_widget, "Неверное значение поля Амплитуда сигнала." });
		id_and_widget_and_error.push_back({ "freq", freq_widget, "Неверное значение поля Частота сигнала." });
	}

	std::map<QString, double> args;

	for (auto [id, widget, error_msg] : id_and_widget_and_error) {
		bool ok = false;
		double arg = widget->text().toDouble(&ok);
		if (!ok) {
			QMessageBox::warning(this, "Неверное значение", error_msg);
			return;
		}
		args[id] = arg;
	}

	if (Speed_checkbox->isChecked()) {
		auto [Q_m3s, Q_litrmin, S] = calc_q_Speed(args["Dp"], args["Ds"], args["Speed"]);

		result_S->setText(QString::number(S, 'g', 10));
        result_Qmin->setText(QString::number(Q_litrmin, 'g', 9));
        result_Qsek->setText(QString::number(Q_m3s, 'g', 9));
	} 
	else {
		auto [Q_m3s, Q_litrmin, S, V] = calc_q_A(args["Dp"], args["Ds"], args["A"], args["freq"]);

		result_S->setText(QString::number(S, 'g', 10));
        result_Qmin->setText(QString::number(Q_litrmin, 'g', 9));
		result_Qsek->setText(QString::number(Q_m3s, 'g', 9));

		Speed_widget->setText(QString::number(V, 'g', 11));
	}
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
		Speed_widget,
		freq_widget,
		result_Qmin,
		result_Qsek,
		result_S,
		Speed_checkbox
	] = find_widgets();

	settings.beginGroup("fields");
	Dp_widget->setText(settings.value("Dp_widget", "").toString());
	Ds_widget->setText(settings.value("Ds_widget", "").toString());
	A_widget->setText(settings.value("A_widget", "").toString());
	Speed_widget->setText(settings.value("Speed_widget", "").toString());
	freq_widget->setText(settings.value("freq_widget", "").toString());
	//result_Qmin->setText(settings.value("result_Qmin", "").toString());
	//result_Qsek->setText(settings.value("result_Qsek", "").toString());

	Speed_checkbox->setChecked(settings.value("Speed_checkbox", false).toBool());

	enable_speed(Speed_checkbox->isChecked());

	settings.endGroup();
}

void miniapp::enable_speed(bool state)
{
	auto [
		Dp_widget,
			Ds_widget,
			A_widget,
			Speed_widget,
			freq_widget,
			result_Qmin,
			result_Qsek,
			result_S,
			Speed_checkbox
	] = find_widgets();

	A_widget->setEnabled(static_cast<bool>(!state));
	freq_widget->setEnabled(static_cast<bool>(!state));
	Speed_widget->setEnabled(static_cast<bool>(state));
}

QSettings miniapp::create_settings_obj()
{
    return QSettings("q_calc.ini", QSettings::IniFormat);
}
