#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QSqlQuery>
#include <QTimer>
#include <QDirIterator>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include <QItemSelectionModel>
#include <QDir>
using namespace cv;
using namespace std;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QItemSelectionModel *select;
    QItemSelectionModel *select_table2;
    QSqlQuery* query;
    QSqlQuery* query2;
    QSqlQuery* query_have_id;
    QSqlQuery* query_havent_id;
    QSqlTableModel *model;
    QSqlTableModel *model2;
    QTimer* time;
    vector<QString> lis;
    void label_splitting(QString filename, QString &plate_licence, float &x, float &y, float &w, float &h);
    int num{0};
    QString licence_plate;
    float x_,y_,w_,h_;
    QSqlDatabase db2;
    int which_table{0};
    QString machine_tip,machine_fild;
    void save_machine(vector<QString>& input);
    QString location_of_image_return(QString in);
    Rect return_rect(double& x,double& y,double& w,double& h,int& image_cols,int& image_rows);
    int saved_num{0};
    int find_pre_saved_images(QString location);
    int size_of_saved{150};

private:
    Ui::MainWindow *ui;
public slots:
    void add_new_element_to_table();
private slots:
    void on_button_adding_to_database_clicked();
    void on_pushButton_unique_type_clicked();
    void on_pushButton_unique_path_clicked();
    void on_pushButton_main_load_clicked();
    void on_pushButton_pdf_clicked();
    void onTableClicked(const QModelIndex &index);
    void onTable2Clicked(const QModelIndex &index);
    void onTable2Moved(const QModelIndex &index,const QModelIndex &index2);
    void on_pushButton_repeated_plate_clicked();
    void on_pushButton_delete_clicked();
    void on_pushButton_clicked();
    void on_pushButton_add_id_clicked();
    void add_id_clicked(QString machine_TIP, QString machine_SYSTEM, int obj_ID);
    void on_saved_machined(QString machine_TIP, QString machine_SYSTEM, int obj_ID);
};

#endif // MAINWINDOW_H
