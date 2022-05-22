#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QtMultimedia>
#include <QDebug>
#include <QString>
#include <QFileDialog>
#include "multimedia_decode_module/ffmpegmultimedia.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //全屏
    isFullScreen=false;
    //播放速度
    fastrate=1;
    //---------SDL----------
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
//    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);
    window = SDL_CreateWindowFrom((void*)ui->SDLwidget->winId());
    render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    texture = SDL_CreateTexture(render,SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,ui->SDLwidget->width(), ui->SDLwidget->height());

    playthread = new PlayThread;
    thread = new QThread;
    playthread->moveToThread(thread);
    thread->start();

    myPlayList=new QStringList();
    //设置播放列表选中模式 按ctrl多选
    ui->listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    //播放列表双击选中播放
    connect(ui->listWidget,&QListWidget::itemDoubleClicked,this,[=](QListWidgetItem * item){
        qDebug()<<"双击选中播放";
//        playthread->close();
//        QThread::msleep(20);
        for(int i=0;i<ui->listWidget->count();i++){
             ui->listWidget->item(i)->setBackground(QColor("white"));
        }
        item->setBackground(QColor(176,224,230));
        int index=ui->listWidget->row(item);
        playthread->openFile(myPlayList->at(index));
        indexPlay=index;
        emit play();

        if(ismute){
            //设置为静音
            emit volumeChange(0);
            ui->volumeBtn->setIcon(QIcon(":/images/mute.png"));
            qDebug()<<"静音状态";

        }else{

            emit volumeChange(volumenum);
            ui->volumeBtn->setIcon(QIcon(":/images/volume.png"));
        }
        emit fastplay(fastrate);
        ui->fastplay->setText(QString::number(fastrate)+"x");
    });

    //指定视频index播放（上一个下一个）
     connect(this,&MainWindow::playById,this,[=](int index){

        ui->fastplay->setText("1.0x");
        for(int i=0;i<ui->listWidget->count();i++){
             ui->listWidget->item(i)->setBackground(QColor("white"));
        }
         ui->listWidget->item(index)->setBackground(QColor(176,224,230));

         playthread->openFile(myPlayList->at(index));
         indexPlay=index;
         emit play();

         if(ismute){
             //设置为静音
             emit volumeChange(0);
             ui->volumeBtn->setIcon(QIcon(":/images/mute.png"));
         }else{

             emit volumeChange(volumenum);
             ui->volumeBtn->setIcon(QIcon(":/images/volume.png"));
         }
         emit fastplay(fastrate);
         ui->fastplay->setText(QString::number(fastrate)+"x");

     });

    //视频进度条初始化
    connect(playthread,&PlayThread::durationChanged,this,[=](qreal i){
         ui->videoSlider->setRange(0,i);
         QString timer=QTime(0,0,0).addSecs(int(i)).toString(QString::fromLatin1("HH:mm:ss"));
         QString s="/ ";
         timer=s+timer;
         ui->duration_label->setText(timer);
    });
    //视频进度条随播放进度变化
    connect(playthread,&PlayThread::positionChanged,this,[=](qreal i){
        if(ui->videoSlider->ismove){
            ui->videoSlider->setValue(i);
        }
        QString timer=QTime(0,0,0).addSecs(int(i)).toString(QString::fromLatin1("HH:mm:ss"));
        ui->currenttime_label->setText(timer);
    });
    //视频进度条被拖动中
    connect(ui->videoSlider,&CustomSlider::sliderMoved ,this,[=](){
        ui->videoSlider->ismove=false;
    });
    //视频进度条被拖动后释放
    connect(ui->videoSlider,&CustomSlider::sliderReleased,this,[=](){
        emit turnto(ui->videoSlider->value());
        ui->videoSlider->ismove=true;
    });
    //视频进度条单击跳转
      connect(ui->videoSlider,&CustomSlider::costomSliderClicked,this,[=](){
           emit turnto(ui->videoSlider->value());
      });

    //音量控制条
      ismute=false;
      volumenum=0.2;
      ui->volumeSlider->setRange(0,100);
      ui->volumeSlider->setValue(20);
      connect(ui->volumeSlider,&QSlider::sliderReleased,this,[=](){
          volumenum=qreal(ui->volumeSlider->value())/100;
          emit volumeChange(volumenum);
      });
      connect(ui->volumeBtn,&QPushButton::clicked,this,[=](){
          if(ismute){
              emit volumeChange(volumenum);
              ui->volumeBtn->setIcon(QIcon(":/images/volume.png"));
              ismute=false;
          }else{
              //设置为静音
              emit volumeChange(0);
              ui->volumeBtn->setIcon(QIcon(":/images/mute.png"));
              ismute=true;
          }
      });

      connect(this,&MainWindow::volumeChange,playthread,&PlayThread::volumeControl);
    //启动信号
    connect(this,&MainWindow::play,playthread,&PlayThread::dowork);
    //跳转信号
    connect(this,&MainWindow::turnto,playthread,&PlayThread::turnto);

    //全屏播放
    connect(ui->fullscreenBtn,&QPushButton::clicked,this,[=](){
        if(isFullScreen){
            showNormal();
            ui->right_widget->setVisible(true);
            ui->menubar->setVisible(true);
            ui->control_widget->setVisible(true);
            isFullScreen=false;
            ui->fullscreenBtn->setIcon(QIcon(":/images/quanping.png"));
        }else{
            showFullScreen();
            ui->right_widget->setVisible(false);
            ui->menubar->setVisible(false);
            ui->control_widget->setVisible(false);
            isFullScreen=true;
             ui->fullscreenBtn->setIcon(QIcon(":/images/quxiaoquanping.png"));
        }


    });
    //全屏播放显示控制窗口
    connect(ui->SDLwidget,&VideoWidget::showControlWidget,this,[=](){
        if(isFullScreen)
            ui->control_widget->setVisible(true);
    });
    //全屏播放隐藏控制窗口
    connect(ui->SDLwidget,&VideoWidget::hideControlWidget,this,[=](){
        if(isFullScreen)
            ui->control_widget->setVisible(false);
    });
    //文件列表项删除后更新播放列表
    connect(this,&MainWindow::itemRemove,ui->listWidget,[=](){
        int count=ui->listWidget->count();
        if (count > 0)
        {
           for(int i=0;i<count;i++)
            {
                 QString text=QString::number(i+1)+"."+ui->listWidget->item(i)->text().split('.').at(1);
                  qDebug()<<text;
                 ui->listWidget->item(i)->setText(text);
            }
        }
    });

    //收到暂停信号 图标切换
    connect(playthread,&PlayThread::pauseSignal,this,[=](){
        ui->pauseButton->setIcon(QIcon(":/images/play.png"));
    });
    //收到播放信号 图标切换
    connect(playthread,&PlayThread::playSignal,this,[=](){
        ui->pauseButton->setIcon(QIcon(":/images/pause.png"));
    });


//    //播放2视频测试按钮
//    connect(ui->pushButton_3,&QPushButton::clicked,this,[=](){
//        playthread->openFile("E:/test/2.avi");
//        emit play();
//    });
    //播放暂停按钮
    connect(ui->pauseButton,&QPushButton::clicked,playthread,&PlayThread::pause);

    QMenu* menu=new QMenu;
    QAction* act05=menu->addAction("0.5x");
    QAction* act10=menu->addAction("1.0x");
    QAction* act125=menu->addAction("1.25x");
    QAction* act15=menu->addAction("1.5x");
    QAction* act20=menu->addAction("2.0x");
    ui->fastplay->setMenu(menu);
    ui->fastplay->setPopupMode(QToolButton::InstantPopup);
   //倍数播放
    connect(act05,&QAction::triggered,this,[=](){
        ui->fastplay->setText("0.5x");
        fastrate=0.5;
        emit fastplay(0.5);
    });
    connect(act10,&QAction::triggered,this,[=](){
        ui->fastplay->setText("1.0x");
        fastrate=1.0;
        emit fastplay(1);
    });
    connect(act125,&QAction::triggered,this,[=](){
        ui->fastplay->setText("1.25x");
        fastrate=1.25;
        emit fastplay(1.25);
    });
    connect(act15,&QAction::triggered,this,[=](){
        ui->fastplay->setText("1.5x");
        fastrate=1.5;
        emit fastplay(1.5);
    });
    connect(act20,&QAction::triggered,this,[=](){
        ui->fastplay->setText("2.0x");
        fastrate=2.0;
        emit fastplay(2);
    });

//    connect(ui->fastplay,&QToolButton::clicked,this,[=]()
//    {
//        emit fastplay(2);
//    });
    connect(this,&MainWindow::fastplay,playthread,&PlayThread::fastplay);
    connect(ui->fastback,&QPushButton::clicked,playthread,&PlayThread::fastback);
    connect(ui->fastforward,&QPushButton::clicked,playthread,&PlayThread::fastforward);

    //播放界面,每秒30帧
    connect(playthread,&PlayThread::updateImage,this,[=](QImage img)
    {
        test = img;
    });
    timer = new QTimer();
    timer->setInterval(33);
    connect(timer,SIGNAL(timeout()),this,SLOT(repaint()));
    timer->start();

}

MainWindow::~MainWindow()
{
    thread->quit();
    thread->wait();
    delete ui;
}
 //键盘按下事件
void MainWindow::keyPressEvent(QKeyEvent *event){
    if(event->key()==Qt::Key_Escape){
        showNormal();
        ui->right_widget->setVisible(true);
        ui->menubar->setVisible(true);
        ui->control_widget->setVisible(true);
        isFullScreen=false;
        ui->fullscreenBtn->setIcon(QIcon(":/images/quanping.png"));
    }
}
//键盘松开事件
void MainWindow::keyReleaseEvent(QKeyEvent *event){

}

void MainWindow::paintEvent(QPaintEvent* pev)
{
    ui->SDLwidget->setUpdatesEnabled(false);
    if(!render){
        qDebug() <<"render not create";
    }else{
        int texture_w=0, texture_h=0;
        SDL_QueryTexture(texture,NULL,NULL,&texture_w,&texture_h);
        if(texture_w != test.width() || texture_h != test.height()){
            if(texture){
                SDL_DestroyTexture(texture);
            }
            texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, test.width(), test.height());
        }
    }
    SDL_UpdateTexture(texture,NULL,test.bits(),test.bytesPerLine());
    SDL_RenderClear(render);
    SDL_RenderCopy(render,texture,NULL,NULL);
    SDL_RenderPresent(render);

}




void MainWindow::on_addFileBtn_clicked()
{
//    qDebug()<<"我被点击了";
    QStringList filenames= QFileDialog::getOpenFileNames(this,"请选择音视频文件","D:/","allfiles(*.*);;""MP3(*.mp3);;""FLV(*.flv)");

    foreach(QString const&str,filenames){
        qDebug()<<str;
        myPlayList->append(str);

        QFileInfo   fileInfo(str);
        int index=ui->listWidget->count();
        QString text=QString::number(index+1)+"."+fileInfo.fileName();
        ui->listWidget->addItem(text);//添加到文件列表

    }
     qDebug()<<"..................";
 //    foreach(QString const&str,myPlayList){
 //         qDebug()<<str;
 //    }
//     for(int i=0;i<myPlayList->count();i++)
//         {
//             QString aFile=myPlayList->at(i);
//             qDebug()<<aFile;
//     }
}


void MainWindow::on_removeFileBtn_clicked()
{
    QList<QListWidgetItem*> items = ui->listWidget->selectedItems();
    if (items.count() > 0)
     {
            foreach(QListWidgetItem* var, items)
            {
                 int index=ui->listWidget->row(var);
                 myPlayList->removeAt(index);
                 ui->listWidget->removeItemWidget(var);
                 items.removeOne(var);
                 delete var;
            }
//             for(int i=0;i<myPlayList->count();i++)
//                 {
//                     QString aFile=myPlayList->at(i);
//                     qDebug()<<aFile;
//             }
      }
    emit itemRemove();
}


void MainWindow::on_lastBtn_clicked()
{
    int count=myPlayList->count();
    if(count>0){
        int last=((indexPlay-1)+count)%count;
        emit playById(last);
    }
}


void MainWindow::on_nextBtn_clicked()
{
    int count=myPlayList->count();
    if(count>0){
        int next=(indexPlay+1)%count;
             emit playById(next);
    }
}

