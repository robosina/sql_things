#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStandardItemModel>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QSqlDatabase db;
    db=QSqlDatabase::addDatabase("QMYSQL","new_s");
    db.setHostName("localhost");
    db.setDatabaseName("cars");
    db.setPort(3306);
    db.setUserName("root");
    db.setPassword("123");
    qDebug()<<db.open();
    query=new QSqlQuery(db);
    query->prepare("SELECT pelak,system,tip FROM cars WHERE pelak=:pel");

    db2=QSqlDatabase::addDatabase("QMYSQL","new_connection");
    db2.setHostName("localhost");
    db2.setDatabaseName("files");
    db2.setPort(3306);
    db2.setUserName("root");
    db2.setPassword("123");
    qDebug()<<db2.open();


    query2=new QSqlQuery(db2);
    //    query2->prepare("INSERT INTO `files`.`machine_classification` (`plate_number_field`, `machine_type_field`, `machine_system_field`, `obj_id`, `x`, `y`, `w`, `h`, `location_of_image`) VALUES ('123344', 'sa', 's', '0', '0.9', '0.6', '0.6', '0.2', 'c/user');");
    query2->prepare("INSERT INTO `files`.`machine_classification` (`plate_number_field`, `machine_type_field`, `machine_system_field`, `obj_id`, `x`, `y`, `w`, `h`, `location_of_image`,`parent_location`) VALUES (:pel, :type, :sys, :Oid, :xval, :yval, :wval, :hval, :path,:par);");
    //    query2->exec();

    model = new QSqlTableModel(parent,db2);
    model2= new QSqlTableModel;

    model->setTable("machine_classification");
    model->select(); //< fetch data


    ui->tableView->setModel(model);
    ui->tableView->setColumnWidth(0,200);
    ui->tableView->setColumnWidth(1,200);
    ui->tableView->setColumnWidth(2,200);
    ui->tableView->setColumnWidth(8,300);
    ui->tableView->setColumnWidth(9,300);
    ui->tableView->setColumnWidth(10,200);
    ui->tableView->setColumnWidth(11,200);


    time=new QTimer;
    connect(time,SIGNAL(timeout()),this,SLOT(add_new_element_to_table()));
    connect(ui->tableView,SIGNAL(clicked(const QModelIndex &)),this,SLOT(onTableClicked(const QModelIndex &)));
    connect(ui->tableView_2,SIGNAL(clicked(const QModelIndex &)),this,SLOT(onTable2Clicked(const QModelIndex &)));

    select=ui->tableView->selectionModel();
    //    connect(select,SIGNAL(currentRowChanged(const QModelIndex &,const QModelIndex &)),this,SLOT(onTableClicked(const QModelIndex &)));

    select_table2=ui->tableView_2->selectionModel();
    connect(select_table2,SIGNAL(currentRowChanged(const QModelIndex &,const QModelIndex &)),this,SLOT(onTable2Moved(const QModelIndex &,const QModelIndex &)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::label_splitting(QString filename,QString& plate_licence,float&x,float&y,float&w,float&h)
{
    QFile file(filename);
    file.open(QIODevice::ReadWrite);
    QTextStream in2(&file);
    while(!in2.atEnd())
    {
        QString line=in2.readLine();
        QStringList list = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        if(list.size()==5)
        {
            x=QString(list[1]).toDouble();
            y=QString(list[2]).toDouble();
            w=QString(list[3]).toDouble();
            h=QString(list[4]).toDouble();
            plate_licence=list[0];
        }
    }
}


void MainWindow::onTableClicked(const QModelIndex &index)
{
    printf("\033[1;31m Table Clicked \n \033[0m ");
    if(index.isValid())
    {
        QString cellText=index.data().toString();
        qDebug()<<cellText;
        qDebug()<<"column="+QString::number(index.column());
        if(which_table==0)
        {
            int row_number=index.row();
            QString data=ui->tableView->model()->data(ui->tableView->model()->index(row_number,8)).toString();
            double x=ui->tableView->model()->data(ui->tableView->model()->index(row_number,4)).toDouble();
            double y=ui->tableView->model()->data(ui->tableView->model()->index(row_number,5)).toDouble();
            double w=ui->tableView->model()->data(ui->tableView->model()->index(row_number,6)).toDouble();
            double h=ui->tableView->model()->data(ui->tableView->model()->index(row_number,7)).toDouble();

            QStringList list=data.split("/");
            QString image_name="";
            for (int i = 0; i < list.size()-1; ++i) {
                image_name+=list[i]+"/";
            }
            list=QString(list[list.size()-1]).split(".");
            image_name=image_name+list[0]+".jpg";
            Mat img=imread(image_name.toStdString());
            x=x-w/2;
            y=y-h/2;
            x*=img.cols;
            y*=img.rows;
            w*=img.cols;
            h*=img.rows;
            cv::rectangle(img,Rect(x,y,w,h),Scalar(0,255,0),3,1);
            cvtColor(img,img,cv::COLOR_BGR2RGB);
            QImage image((uchar*) img.data,img.cols,img.rows,QImage::Format_RGB888);
            ui->label->setPixmap(QPixmap::fromImage(image));
            ui->label->setScaledContents(true);
            qDebug()<<"data="<<image_name;
        }
        else if(which_table==1)
        {
            int row_number=index.row();
            QString machine_type=ui->tableView->model()->data(ui->tableView->model()->index(row_number,0)).toString();
            QString machine_field=ui->tableView->model()->data(ui->tableView->model()->index(row_number,1)).toString();
            machine_tip=machine_type;
            machine_fild=machine_field;
            ui->lineEdit_tip->setText(machine_tip);
            ui->lineEdit_system->setText(machine_field);
            ui->lineEdit_objid->setText(ui->tableView->model()->data(ui->tableView->model()->index(row_number,3)).toString());
            ui->pushButton->setText(machine_tip+" "+machine_fild);
            QString qury_filter=QString("SELECT * FROM `machine_classification` WHERE (`machine_type_field`='%1' AND `machine_system_field`='%2')").arg(machine_type).arg(machine_field);
            QSqlQuery sql(qury_filter,db2);
            sql.exec();
            model2->clear();
            //    model = new QSqlTableModel(this,db2);
            vector<QString> fields_value;
            fields_value.push_back("plate_number_field");
            fields_value.push_back("machine_type_field");
            fields_value.push_back("machine_system_field");
            fields_value.push_back("obj_id");
            fields_value.push_back("x");
            fields_value.push_back("y");
            fields_value.push_back("w");
            fields_value.push_back("h");
            fields_value.push_back("location_of_image");
            fields_value.push_back("parent_location");
            fields_value.push_back("size_of_saved");
            fields_value.push_back("check_repeated");
            fields_value.push_back("is_saved");
            fields_value.push_back("address_of_saved_car");
            QStandardItemModel *model1 = new QStandardItemModel(sql.size(),fields_value.size());
            for (int i = 0; i < fields_value.size(); ++i)
            {
                model1->setHeaderData(i, Qt::Horizontal, QObject::tr(fields_value[i].toStdString().c_str()));
            }

            int row=0;
            while(sql.next())
            {
                for (int i = 0; i < fields_value.size(); ++i)
                {
                    QModelIndex index=model1->index(row,i);
                    QString type=sql.value(i).toString();
                    model1->setData(index,type,Qt::EditRole);
                }
                row++;
            }
            ui->tableView_2->setModel(model1);
            ui->tableView_2->setColumnWidth(13,300);
        }
        else if(which_table==3)
        {
            int row_number=index.row();
            QString plate_number=ui->tableView->model()->data(ui->tableView->model()->index(row_number,2)).toString();

            QString qury_filter=QString("SELECT * FROM machine_classification WHERE plate_number_field=%1").arg(plate_number);
            QSqlQuery sql(qury_filter,db2);
            sql.exec();
            model2->clear();
            vector<QString> fields_value;
            fields_value.push_back("plate_number_field");
            fields_value.push_back("machine_type_field");
            fields_value.push_back("machine_system_field");
            fields_value.push_back("obj_id");
            fields_value.push_back("x");
            fields_value.push_back("y");
            fields_value.push_back("w");
            fields_value.push_back("h");
            fields_value.push_back("location_of_image");
            fields_value.push_back("parent_location");
            fields_value.push_back("size_of_saved");
            fields_value.push_back("check_repeated");
            fields_value.push_back("is_saved");
            fields_value.push_back("address_of_saved_car");
            QStandardItemModel *model1 = new QStandardItemModel(sql.size(),fields_value.size());
            for (int i = 0; i < fields_value.size(); ++i)
            {
                model1->setHeaderData(i, Qt::Horizontal, QObject::tr(fields_value[i].toStdString().c_str()));
            }

            int row=0;
            while(sql.next())
            {
                for (int i = 0; i < fields_value.size(); ++i)
                {
                    QModelIndex index=model1->index(row,i);
                    QString type=sql.value(i).toString();
                    model1->setData(index,type,Qt::EditRole);
                }
                row++;
            }
            ui->tableView_2->setModel(model1);
            ui->tableView->setColumnWidth(10,300);
        }
    }
}

void MainWindow::onTable2Clicked(const QModelIndex &index)
{
    qDebug()<<"clicked";
    int row_number=index.row();
    QString data=ui->tableView_2->model()->data(ui->tableView_2->model()->index(row_number,8)).toString();
    double x=ui->tableView_2->model()->data(ui->tableView_2->model()->index(row_number,4)).toDouble();
    double y=ui->tableView_2->model()->data(ui->tableView_2->model()->index(row_number,5)).toDouble();
    double w=ui->tableView_2->model()->data(ui->tableView_2->model()->index(row_number,6)).toDouble();
    double h=ui->tableView_2->model()->data(ui->tableView_2->model()->index(row_number,7)).toDouble();

    QStringList list=data.split("/");
    QString image_name="";
    for (int i = 0; i < list.size()-1; ++i) {
        image_name+=list[i]+"/";
    }
    list=QString(list[list.size()-1]).split(".");
    image_name=image_name+list[0]+".jpg";
    Mat img=imread(image_name.toStdString());
    x=x-w/2;
    y=y-h/2;
    x*=img.cols;
    y*=img.rows;
    w*=img.cols;
    h*=img.rows;
    cv::rectangle(img,Rect(x,y,w,h),Scalar(0,255,0),3,1);
    cvtColor(img,img,cv::COLOR_BGR2RGB);
    QImage image((uchar*) img.data,img.cols,img.rows,QImage::Format_RGB888);
    ui->label->setPixmap(QPixmap::fromImage(image));
    ui->label->setScaledContents(true);
    qDebug()<<"data="<<image_name;
}

void MainWindow::onTable2Moved(const QModelIndex &index, const QModelIndex &index2)
{
    qDebug()<<"clicked";
    int row_number=index.row();
    QString data=ui->tableView_2->model()->data(ui->tableView_2->model()->index(row_number,8)).toString();
    double x=ui->tableView_2->model()->data(ui->tableView_2->model()->index(row_number,4)).toDouble();
    double y=ui->tableView_2->model()->data(ui->tableView_2->model()->index(row_number,5)).toDouble();
    double w=ui->tableView_2->model()->data(ui->tableView_2->model()->index(row_number,6)).toDouble();
    double h=ui->tableView_2->model()->data(ui->tableView_2->model()->index(row_number,7)).toDouble();

    QStringList list=data.split("/");
    QString image_name="";
    for (int i = 0; i < list.size()-1; ++i) {
        image_name+=list[i]+"/";
    }
    list=QString(list[list.size()-1]).split(".");
    image_name=image_name+list[0]+".jpg";
    Mat img=imread(image_name.toStdString());
    x=x-w/2;
    y=y-h/2;
    x*=img.cols;
    y*=img.rows;
    w*=img.cols;
    h*=img.rows;
    cv::rectangle(img,Rect(x,y,w,h),Scalar(0,255,0),3,1);
    cvtColor(img,img,cv::COLOR_BGR2RGB);
    QImage image((uchar*) img.data,img.cols,img.rows,QImage::Format_RGB888);
    ui->label->setPixmap(QPixmap::fromImage(image));
    ui->label->setScaledContents(true);
    qDebug()<<"data="<<image_name;
}

void MainWindow::add_new_element_to_table()
{
    time->stop();
    QString file_name=lis[num];
    label_splitting(file_name,licence_plate,x_,y_,w_,h_);
    qDebug()<<num<<" of "<<lis.size();
    query->bindValue(":pel",licence_plate);
    query->exec();
    if(query->size()>0)
    {
        query->first();
        QString system=query->value(2).toString();
        QString tip=query->value(1).toString();
        query2->bindValue(":pel",licence_plate.toInt());
        query2->bindValue(":type",tip.toUtf8());
        query2->bindValue(":sys",system.toUtf8());
        query2->bindValue(":Oid",(int) 0);
        query2->bindValue(":xval",x_);
        query2->bindValue(":yval",y_);
        query2->bindValue(":wval",w_);
        query2->bindValue(":hval",h_);
        query2->bindValue(":path",lis[num]);
        QStringList lis_eq=QString(lis[num]).split("/");
        QString parent_string;
        for (int i = 0; i < lis_eq.size()-1; ++i) {
            parent_string+=lis_eq[i]+"/";
        }
        query2->bindValue(":par",parent_string);
        query2->exec();
    }
    num++;
    if(num>=lis.size())
    {

    }
    else
    {
        time->start(1);
    }
}

void MainWindow::on_button_adding_to_database_clicked()
{
    num=0;
    QDirIterator it(ui->lineEdit->text(), QStringList() << "*.txt*", QDir::Files, QDirIterator::Subdirectories);
    lis.resize(0);
    while(it.hasNext())
    {
        lis.push_back(it.next());
    }

    time->start(1);
}

void MainWindow::on_pushButton_unique_type_clicked()
{
    which_table=1;
    //    SELECT COUNT(DISTINCT machine_type_field ) AS some_alias FROM machine_classification
    //    QSqlQuery query3;
    //    query3.prepare("SELECT DISTINCT machine_type_field FROM  `machine_classification` ");
    //    SELECT machine_type_field , COUNT(*) FROM machine_classification GROUP BY machine_type_field  HAVING COUNT(*) >= 1
    //    query3.exec();
    QString qury_filter="SELECT machine_type_field,`machine_system_field` ,COUNT(*),obj_id FROM machine_classification GROUP BY machine_type_field,`machine_system_field`  HAVING COUNT(*) >= 100 ORDER BY obj_id DESC";
    //QString qury_filter="SELECT machine_type_field , COUNT(*) FROM machine_classification GROUP BY machine_type_field  HAVING COUNT(*) >= 50 ORDER BY COUNT( * ) DESC";
    QSqlQuery sql(qury_filter,db2);
    sql.exec();
    model->clear();
    //    model = new QSqlTableModel(this,db2);
    QStandardItemModel *model1 = new QStandardItemModel(sql.size(),8);
    model1->setHeaderData(0, Qt::Horizontal, QObject::tr("type"));
    model1->setHeaderData(1, Qt::Horizontal, QObject::tr("System"));
    model1->setHeaderData(2, Qt::Horizontal, QObject::tr("Count"));
    model1->setHeaderData(3, Qt::Horizontal, QObject::tr("Obj_id"));
    model1->setHeaderData(4, Qt::Horizontal, QObject::tr("HAVE OBJ ID"));
    model1->setHeaderData(5, Qt::Horizontal, QObject::tr("HAVEN't OBJ ID"));
    model1->setHeaderData(6, Qt::Horizontal, QObject::tr("n of saved"));
    model1->setHeaderData(7, Qt::Horizontal, QObject::tr("n of not saved"));

    int row=0;
    while(sql.next())
    {
        QModelIndex index=model1->index(row,0);
        QString type=sql.value(0).toString();
        QString system=sql.value(1).toString();
        QString count=sql.value(2).toString();
        QString obj_id=sql.value(3).toString();
        model1->setData(index,type,Qt::EditRole);
        QModelIndex index2=model1->index(row,1);
        model1->setData(index2,system,Qt::EditRole);
        QModelIndex index3=model1->index(row,2);
        model1->setData(index3,count,Qt::EditRole);
        QModelIndex index4=model1->index(row,3);
        model1->setData(index4,obj_id,Qt::EditRole);

        QString qury_filter2=QString("SELECT machine_type_field,`machine_system_field` ,COUNT(*),obj_id FROM machine_classification WHERE (`machine_type_field`='%1' AND `machine_system_field`='%2' AND `obj_id`=0) GROUP BY machine_type_field,`machine_system_field`  HAVING COUNT(*) > 1").arg(type).arg(system);
        QSqlQuery sql2(qury_filter2,db2);
        sql2.exec();
        sql2.next();
        QModelIndex index5=model1->index(row,5);
        model1->setData(index5,sql2.value(2).toString());
        if(sql2.value(2).toInt()>0)
        {
            add_id_clicked(type,system,obj_id.toInt());
        }
        QModelIndex index6=model1->index(row,4);
        model1->setData(index6,count.toInt()-sql2.value(2).toInt());

        QString query_filter3=QString("SELECT machine_type_field,`machine_system_field` ,COUNT(*),obj_id,`is_saved` FROM machine_classification WHERE (`machine_type_field`='%1' AND `machine_system_field`='%2' AND `is_saved`=0) GROUP BY machine_type_field,`machine_system_field`  HAVING COUNT(*) > 1").arg(type).arg(system);
        QSqlQuery sql3(query_filter3,db2);
        sql3.exec();
        sql3.next();

        QModelIndex index7=model1->index(row,7);
        model1->setData(index7,sql3.value(2).toString());

        if(sql3.value(2).toInt()>0 & obj_id.toInt()>0 & ui->checkBox_per_to_save->isChecked())
        {
            on_saved_machined(type,system,obj_id.toInt());
        }
        QModelIndex index8=model1->index(row,6);
        model1->setData(index8,count.toInt()-sql3.value(2).toInt());

        row++;
    }
    ui->tableView->setModel(model1);

    ui->tableView->setColumnWidth(0,150);
    ui->tableView->setColumnWidth(1,150);
    ui->tableView->setColumnWidth(4,150);
    ui->tableView->setColumnWidth(5,150);
    ui->tableView->setColumnWidth(6,150);
    ui->tableView->setColumnWidth(7,150);
}

void MainWindow::on_pushButton_unique_path_clicked()
{
    which_table=2;
    QString qury_filter="SELECT parent_location , COUNT(*) FROM machine_classification GROUP BY parent_location  HAVING COUNT(*) >= 1";
    QSqlQuery sql(qury_filter,db2);
    sql.exec();
    model->clear();
    //    model = new QSqlTableModel(this,db2);
    QStandardItemModel *model1 = new QStandardItemModel(sql.size(),2);
    model1->setHeaderData(0, Qt::Horizontal, QObject::tr("parent_path"));
    model1->setHeaderData(1, Qt::Horizontal, QObject::tr("count"));
    int row=0;
    while(sql.next())
    {
        QModelIndex index=model1->index(row,0);
        QString type=sql.value(0).toString();
        QString count=sql.value(1).toString();
        model1->setData(index,type,Qt::EditRole);
        QModelIndex index2=model1->index(row,1);
        model1->setData(index2,count,Qt::EditRole);
        row++;
    }
    ui->tableView->setModel(model1);
    ui->tableView->setColumnWidth(0,400);
}

void MainWindow::on_pushButton_main_load_clicked()
{
    which_table=0;
    model = new QSqlTableModel(this,db2);
    model->setTable("machine_classification");
    model->select(); //< fetch data
    ui->tableView->setModel(model);

    ui->tableView->setColumnWidth(8,300);
    ui->tableView->setColumnWidth(9,300);
}

void MainWindow::on_pushButton_pdf_clicked()
{

}

void MainWindow::on_pushButton_repeated_plate_clicked()
{
    which_table=3;
    QString qury_filter="SELECT machine_type_field,machine_system_field,plate_number_field , COUNT(*) FROM machine_classification GROUP BY plate_number_field  HAVING COUNT(*) >= 60 ORDER BY COUNT( * ) DESC";
    QSqlQuery sql(qury_filter,db2);
    sql.exec();
    model->clear();
    //    model = new QSqlTableModel(this,db2);
    QStandardItemModel *model1 = new QStandardItemModel(sql.size(),4);
    model1->setHeaderData(0, Qt::Horizontal, QObject::tr("type"));
    model1->setHeaderData(1, Qt::Horizontal, QObject::tr("System"));
    model1->setHeaderData(2, Qt::Horizontal, QObject::tr("plate_number_field"));
    model1->setHeaderData(3, Qt::Horizontal, QObject::tr("count"));
    int row=0;
    while(sql.next())
    {
        QModelIndex index=model1->index(row,0);
        QString type=sql.value(0).toString();
        QString system=sql.value(1).toString();
        QString plate_number_field=sql.value(2).toString();
        QString count=sql.value(3).toString();
        model1->setData(index,type,Qt::EditRole);
        QModelIndex index2=model1->index(row,1);
        model1->setData(index2,system,Qt::EditRole);
        QModelIndex index3=model1->index(row,2);
        model1->setData(index3,plate_number_field,Qt::EditRole);
        QModelIndex index4=model1->index(row,3);
        model1->setData(index4,count,Qt::EditRole);
        row++;
    }
    ui->tableView->setModel(model1);
    ui->tableView->setColumnWidth(0,250);
    ui->tableView->setColumnWidth(1,250);
    ui->tableView->setColumnWidth(2,250);
}

void MainWindow::on_pushButton_delete_clicked()
{
    which_table=3;
    QString qury_filter="SELECT machine_type_field,machine_system_field,plate_number_field , COUNT(*) FROM machine_classification GROUP BY plate_number_field  HAVING COUNT(*) >= 60 ORDER BY COUNT( * ) DESC";
    QSqlQuery sql(qury_filter,db2);
    sql.exec();
    model->clear();
    //    model = new QSqlTableModel(this,db2);
    QStandardItemModel *model1 = new QStandardItemModel(sql.size(),4);
    model1->setHeaderData(0, Qt::Horizontal, QObject::tr("type"));
    model1->setHeaderData(1, Qt::Horizontal, QObject::tr("System"));
    model1->setHeaderData(2, Qt::Horizontal, QObject::tr("plate_number_field"));
    model1->setHeaderData(3, Qt::Horizontal, QObject::tr("count"));
    int row=0;
    while(sql.next())
    {
        QModelIndex index=model1->index(row,0);
        QString type=sql.value(0).toString();
        QString system=sql.value(1).toString();
        QString plate_number_field=sql.value(2).toString();
        QString qury_filter2=QString("DELETE FROM `machine_classification` WHERE `plate_number_field`=%1").arg(plate_number_field);
        QSqlQuery sql2(qury_filter2,db2);
        sql2.exec();
        QString count=sql.value(3).toString();
        model1->setData(index,type,Qt::EditRole);
        QModelIndex index2=model1->index(row,1);
        model1->setData(index2,system,Qt::EditRole);
        QModelIndex index3=model1->index(row,2);
        model1->setData(index3,plate_number_field,Qt::EditRole);
        QModelIndex index4=model1->index(row,3);
        model1->setData(index4,count,Qt::EditRole);
        row++;
    }
    ui->tableView->setModel(model1);
    ui->tableView->setColumnWidth(0,250);
    ui->tableView->setColumnWidth(1,250);
    ui->tableView->setColumnWidth(2,250);

}

void MainWindow::on_pushButton_clicked()
{
    QString qury_filter=QString("SELECT obj_id,x,y,w,h,location_of_image,is_saved FROM `machine_classification` WHERE (`machine_type_field`='%1' AND `machine_system_field`='%2')").arg(machine_tip).arg(machine_fild);
    QSqlQuery sql(qury_filter,db2);
    sql.exec();
    model2->clear();
    //    model = new QSqlTableModel(this,db2);
    vector<QString> fields_value;
    fields_value.push_back("obj_id");
    fields_value.push_back("x");
    fields_value.push_back("y");
    fields_value.push_back("w");
    fields_value.push_back("h");
    fields_value.push_back("location_of_image");
    fields_value.push_back("is_saved");

    vector<QString> values;
    saved_num=0;
    cout<<"total size is="<<sql.size()<<endl;
    int sss=0;
    while(sql.next())
    {
        sss++;
        values.resize(0);
        for (int i = 0; i < fields_value.size(); ++i)
        {
            values.push_back(sql.value(i).toString());
        }
        if(saved_num==0)
        {
            int obj_id=values[0].toInt();
            saved_num=find_pre_saved_images("E:/total_car_classification/final_car/"+QString::number(obj_id));
        }
        save_machine(values);
        cout<<"proceed:"<<sss<<" from "<<sql.size()<<" save num="<<saved_num<<endl;
    }
}

void MainWindow::save_machine(vector<QString> &input)
{
    int obj_id=input[0].toInt();
    if(obj_id>0)
    {
        int is_saved=input[6].toInt();
        QString path_to_folder="E:/total_car_classification/final_car/"+QString::number(obj_id);
        if(is_saved==0)
        {
            if(!QDir(path_to_folder).exists())
            {
                QDir().mkdir(path_to_folder);
            }
            double x=input[1].toDouble();
            double y=input[2].toDouble();
            double w=input[3].toDouble();
            double h=input[4].toDouble();
            QString location_of_image=input[5];
            QString lll=location_of_image;
            location_of_image=location_of_image_return(location_of_image);
            Mat img=imread(location_of_image.toStdString());
            Rect r=return_rect(x,y,w,h,img.cols,img.rows);
            Mat car=img(r);
            try
            {
                cv::resize(car,car,Size(size_of_saved,size_of_saved));
            }
            catch(cv::Exception& err)
            {
                cout<<"error"<<endl;
            }
            QString address_of_saved_car=path_to_folder+"/"+QString::number(saved_num)+".jpg";
            bool is_image_saved=imwrite(address_of_saved_car.toStdString(),car);
            if(is_image_saved)
            {
                saved_num++;
                QString qury_filter=QString("UPDATE `machine_classification` SET `size_of_saved`=%3,`is_saved`=1,`address_of_saved_car`='%1' WHERE `location_of_image`='%2'").arg(address_of_saved_car).arg(lll).arg(size_of_saved);
                QSqlQuery sql(qury_filter,db2);
                sql.exec();
            }
        }
    }
}

QString MainWindow::location_of_image_return(QString in)
{
    QStringList sep1=in.split("/");
    QString text_name=sep1[sep1.size()-1];
    QStringList sep2=text_name.split(".");
    QString image_part=sep2[0];
    QString image_name="";
    for (int i = 0; i < sep1.size()-1; ++i)
    {
        image_name+=sep1[i]+"/";
    }
    image_name+=image_part+".jpg";
    return image_name;
}

Rect MainWindow::return_rect(double &x, double &y, double &w, double &h, int &image_cols, int &image_rows)
{
    x=x-w/2;
    y=y-h/2;
    x*=image_cols;
    y*=image_rows;
    w*=image_cols;
    h*=image_rows;
    Rect r=Rect(1,1,image_cols-1,image_rows-1);
    r=r& Rect(x,y,w,h);
    return r;
}

int MainWindow::find_pre_saved_images(QString location)
{
    int i=1;
    QFile file(location+"/"+QString::number(i)+".jpg");
    while(file.exists())
    {
        file.setFileName(location+"/"+QString::number(i)+".jpg");
        i++;
    }
    if(i==0)
    {
        return 0;
    }
    else
    {
        return i-1;
    }
}

void MainWindow::on_pushButton_add_id_clicked()
{
    QString obj_id=ui->lineEdit_objid->text();
    if(obj_id.length()>0)
    {
        int o_id=obj_id.toInt();
        if(o_id>0)
        {
            QString sql_command=QString("UPDATE machine_classification  SET `obj_id`=%1 WHERE (`machine_type_field`='%2' AND `machine_system_field`='%3' AND `obj_id`=0)").arg(o_id).arg(ui->lineEdit_tip->text()).arg(ui->lineEdit_system->text());
            qDebug()<<sql_command;
            QSqlQuery sql(sql_command,db2);
            sql.exec();
        }
    }
}

void MainWindow::add_id_clicked(QString machine_TIP,QString machine_SYSTEM,int obj_ID)
{
    if(obj_ID>0)
    {
        QString sql_command=QString("UPDATE machine_classification  SET `obj_id`=%1 WHERE (`machine_type_field`='%2' AND `machine_system_field`='%3' AND `obj_id`=0)").arg(obj_ID).arg(machine_TIP).arg(machine_SYSTEM);
        qDebug()<<sql_command;
        QSqlQuery sql(sql_command,db2);
        sql.exec();
    }
}

void MainWindow::on_saved_machined(QString machine_TIP, QString machine_SYSTEM, int obj_ID)
{
    QString qury_filter=QString("SELECT obj_id,x,y,w,h,location_of_image,is_saved FROM `machine_classification` WHERE (`machine_type_field`='%1' AND `machine_system_field`='%2')").arg(machine_TIP).arg(machine_SYSTEM);
    QSqlQuery sql(qury_filter,db2);
    sql.exec();
    model2->clear();
    //    model = new QSqlTableModel(this,db2);
    vector<QString> fields_value;
    fields_value.push_back("obj_id");
    fields_value.push_back("x");
    fields_value.push_back("y");
    fields_value.push_back("w");
    fields_value.push_back("h");
    fields_value.push_back("location_of_image");
    fields_value.push_back("is_saved");

    vector<QString> values;
    saved_num=0;
    cout<<"total size is="<<sql.size()<<endl;
    int sss=0;
    while(sql.next())
    {
        sss++;
        values.resize(0);
        for (int i = 0; i < fields_value.size(); ++i)
        {
            values.push_back(sql.value(i).toString());
        }
        if(saved_num==0)
        {
            int obj_id=values[0].toInt();
            saved_num=find_pre_saved_images("E:/total_car_classification/final_car/"+QString::number(obj_id));
        }
        save_machine(values);
        cout<<"proceed:"<<sss<<" from "<<sql.size()<<" save num="<<saved_num<<endl;
    }
}
