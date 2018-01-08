/*  StarProfileViewer
    Copyright (C) 2017 Robert Lancaster <rlancaste@gmail.com>

    Based on the QT Surface Example http://doc.qt.io/qt-5.9/qtdatavisualization-surface-example.html
    and the QT Bars Example https://doc-snapshots.qt.io/qt5-5.9/qtdatavisualization-bars-example.html

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
*/

#include "starprofileviewer.h"

using namespace QtDataVisualization;

StarProfileViewer::StarProfileViewer(QWidget *parent) : QDialog(parent)
{

#ifdef Q_OS_OSX
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
#endif

    m_graph = new Q3DBars();
    m_pixelValueAxis = m_graph->valueAxis();
    m_xPixelAxis = m_graph->columnAxis();
    m_yPixelAxis = m_graph->rowAxis();

    m_pixelValueAxis->setTitle("Pixel Values");
    m_pixelValueAxis->setLabelAutoRotation(30.0f);
    m_pixelValueAxis->setTitleVisible(true);

    m_xPixelAxis->setTitle("Horizontal");
    m_xPixelAxis->setLabelAutoRotation(30.0f);
    m_xPixelAxis->setTitleVisible(true);
    m_yPixelAxis->setTitle("Vertical");
    m_yPixelAxis->setLabelAutoRotation(30.0f);
    m_yPixelAxis->setTitleVisible(true);

    m_3DPixelSeries = new QBar3DSeries;

    m_3DPixelSeries->setMesh(QAbstract3DSeries::MeshBevelBar);
    m_graph->addSeries(m_3DPixelSeries);

    m_graph->activeTheme()->setLabelBackgroundEnabled(false);

    QWidget *container = QWidget::createWindowContainer(m_graph);

    if (!m_graph->hasContext()) {
        QMessageBox msgBox;
        msgBox.setText("Couldn't initialize the OpenGL context.");
        msgBox.exec();
        return;
    }

    QSize screenSize = m_graph->screen()->size();
    container->setMinimumSize(QSize(300, 500));
    container->setMaximumSize(screenSize);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container->setFocusPolicy(Qt::StrongFocus);

    this->setWindowTitle(QStringLiteral("View Star Profile"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *topLayout = new QHBoxLayout();
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    QWidget* rightWidget = new QWidget();
    rightWidget->setVisible(false);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    QHBoxLayout *sliderLayout = new QHBoxLayout();

    topLayout->addWidget(container, 1);
    topLayout->addWidget(rightWidget);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);
    bottomLayout->setAlignment(Qt::AlignLeft);

    maxValue=new QLabel(this);
    maxValue->setToolTip("Maximum Value on the graph");
    cutoffValue=new QLabel(this);
    cutoffValue->setToolTip("Cuttoff Maximum for eliminating hot pixels and bright stars.");

    QCheckBox *toggleEnableCutoff= new QCheckBox(this);
    toggleEnableCutoff->setToolTip("Enable or Disable the Max Value Cutoff");
    toggleEnableCutoff->setText("Toggle Cutoff");
    toggleEnableCutoff->setChecked(false);

    blackPointSlider=new QSlider( Qt::Vertical, this);
    blackPointSlider->setToolTip("Sets the Minimum Value on the graph");
    sliderLayout->addWidget(blackPointSlider);

    whitePointSlider=new QSlider( Qt::Vertical, this);
    whitePointSlider->setToolTip("Sets the Maximum Value on the graph");
    sliderLayout->addWidget(whitePointSlider);

    cutoffSlider=new QSlider( Qt::Vertical, this);
    cutoffSlider->setToolTip("Sets the Cuttoff Maximum for eliminating hot pixels and bright stars.");
    sliderLayout->addWidget(cutoffSlider);
    cutoffSlider->setEnabled(false);

    minValue = new QLabel(this);
    minValue->setToolTip("Minimum Value on the graph");

    autoScale = new QCheckBox(this);
    autoScale->setText("AutoScale");
    autoScale->setToolTip("Automatically scales the sliders for the subFrame.\nUncheck to leave them unchanged when you pan around.");
    autoScale->setChecked(true);

    showScaling = new QCheckBox(this);
    showScaling->setText("Scaling");
    showScaling->setToolTip("Hides and shows the scaling side panel");
    showScaling->setChecked(false);

    rightLayout->addWidget(toggleEnableCutoff);
    rightLayout->addWidget(cutoffValue);
    rightLayout->addWidget(maxValue);
    rightLayout->addLayout(sliderLayout);
    rightLayout->addWidget(new QLabel("Min, Max, Cut     "));
    rightLayout->addWidget(minValue);
    rightLayout->addWidget(autoScale);

    selectionType = new QComboBox(this);
    selectionType->setToolTip("Changes the type of selection");
    selectionType->addItem(QStringLiteral("Item"));
    selectionType->addItem(QStringLiteral("Horizontal"));
    selectionType->addItem(QStringLiteral("Vertical"));
    selectionType->setCurrentIndex(0);

    sliceB = new QPushButton(this);
    sliceB->setText(QStringLiteral("Slice"));
    sliceB->setToolTip("Toggles the slice view when horizontal or vertical items are selected");
    sliceB->setCheckable(true);
    sliceB->setChecked(false);
    sliceB->setEnabled(false);
    sliceB->setDefault(false);

    HFRReport = new QCheckBox(this);
    HFRReport->setToolTip("Shows the HFR of stars in the frame");
    HFRReport->setText("HFR: ");
    HFRReport->setChecked(true);
    HFRReportBox = new QLabel(this);

    showPeakValues = new QCheckBox(this);
    showPeakValues->setText("Peak: ");
    showPeakValues->setToolTip("Shows the peak values of stars in the frame");
    showPeakValues->setChecked(true);
    peakValue = new QLabel(this);

    sampleSize = new QComboBox(this);
    sampleSize->setToolTip("Changes the sample size shown in the graph");
    sampleSize->addItem(QString::number(16));
    sampleSize->addItem(QString::number(32));
    sampleSize->addItem(QString::number(64));
    sampleSize->addItem(QString::number(128));
    sampleSize->addItem(QString::number(256));
    sampleSize->addItem(QString::number(512));
    sampleSize->setCurrentIndex(3);
    sampleSize->setVisible(false);

    QComboBox *color = new QComboBox(this);
    color->setToolTip("Changes the color scheme");

    QLinearGradient grGtoR(50, 1, 0, 0);
    grGtoR.setColorAt(1.0, Qt::darkGreen);
    grGtoR.setColorAt(0.5, Qt::yellow);
    grGtoR.setColorAt(0.2, Qt::red);
    grGtoR.setColorAt(0.0, Qt::darkRed);
    QPixmap pm(50, 10);
    QPainter pmp(&pm);
    pmp.setPen(Qt::NoPen);
    pmp.setBrush(QBrush(grGtoR));
    pmp.drawRect(0, 0, 50, 10);
    color->addItem("");
    color->setItemIcon(0,QIcon(pm));

    QLinearGradient grBtoY(50, 1, 0, 0);
    grBtoY.setColorAt(1.0, Qt::black);
    grBtoY.setColorAt(0.67, Qt::blue);
    grBtoY.setColorAt(0.33, Qt::red);
    grBtoY.setColorAt(0.0, Qt::yellow);
    pmp.setBrush(QBrush(grBtoY));
    pmp.drawRect(0, 0, 50, 10);
    color->addItem("");
    color->setItemIcon(1,QIcon(pm));

    color->setIconSize(QSize(50, 10));
    color->setCurrentIndex(0);
    color->setMaximumWidth(80);

    zoomView = new QComboBox(this);
    zoomView->setToolTip("Zooms the view to preset locations.");
    zoomView->addItem(QStringLiteral("ZoomTo:"));
    zoomView->addItem(QStringLiteral("Front"));
    zoomView->addItem(QStringLiteral("Front High"));
    zoomView->addItem(QStringLiteral("Overhead"));
    zoomView->addItem(QStringLiteral("Isometric L"));
    zoomView->addItem(QStringLiteral("Isometric R"));
    zoomView->addItem(QStringLiteral("Selected Item"));
    zoomView->setCurrentIndex(0);

    QCheckBox *selectorsVisible = new QCheckBox(this);
    selectorsVisible->setToolTip("Hides and shows the Vertical and Horizontal Selection Sliders");
    selectorsVisible->setChecked(false);

    bottomLayout->addWidget(sampleSize);
    bottomLayout->addWidget(selectionType);
    bottomLayout->addWidget(selectorsVisible);
    bottomLayout->addWidget(sliceB);
    bottomLayout->addWidget(zoomView);
    bottomLayout->addWidget(color);
    bottomLayout->addWidget(HFRReport);
    bottomLayout->addWidget(HFRReportBox);
    bottomLayout->addWidget(new QLabel(", "));
    bottomLayout->addWidget(showPeakValues);
    bottomLayout->addWidget(peakValue);
    bottomLayout->addWidget(showScaling);

    verticalSelector = new QSlider(Qt::Horizontal, this);
    verticalSelector->setToolTip("Selects the Vertical Value");
    verticalSelector->setVisible(false);
    horizontalSelector = new QSlider(Qt::Horizontal, this);
    horizontalSelector->setToolTip("Selects the Horizontal Value");
    horizontalSelector->setVisible(false);
    exploreMode = new QCheckBox(this);
    exploreMode->setText("Explore Mode");
    exploreMode->setToolTip("Zooms automatically as the sliders change");
    exploreMode->setChecked(true);
    exploreMode->setVisible(false);
    mainLayout->addWidget(verticalSelector);
    mainLayout->addWidget(horizontalSelector);
    mainLayout->addWidget(exploreMode);

    QObject::connect(selectionType,  SIGNAL(currentIndexChanged(int)),
                     this, SLOT(changeSelectionType(int)));
    QObject::connect(zoomView,  SIGNAL(currentIndexChanged(int)),
                     this, SLOT(zoomViewTo(int)));
    QObject::connect(sliceB,  &QPushButton::pressed,
                     this, &StarProfileViewer::toggleSlice);
    QObject::connect(HFRReport,  &QCheckBox::toggled,
                     this, &StarProfileViewer::updateHFRandPeakSelection);
    QObject::connect(showPeakValues,  &QCheckBox::toggled,
                     this, &StarProfileViewer::updateHFRandPeakSelection);
    QObject::connect(blackPointSlider,  &QSlider::valueChanged,
                     this, &StarProfileViewer::updateVerticalAxis);
    QObject::connect(whitePointSlider,  &QSlider::valueChanged,
                     this, &StarProfileViewer::updateVerticalAxis);
    QObject::connect(cutoffSlider,  &QSlider::valueChanged,
                     this, &StarProfileViewer::updateDisplayData);
    QObject::connect(autoScale,  &QCheckBox::toggled,
                     this, &StarProfileViewer::updateScale);
    QObject::connect(showScaling,  &QCheckBox::toggled,
                     rightWidget, &QWidget::setVisible);
    QObject::connect(sampleSize,  SIGNAL(currentIndexChanged(QString)),
                     this, SLOT(updateSampleSize(QString)));
    QObject::connect(color,  SIGNAL(currentIndexChanged(int)),
                     this, SLOT(updateColor(int)));
    QObject::connect(verticalSelector,  &QSlider::valueChanged,
                     this, &StarProfileViewer::changeSelection);
    QObject::connect(horizontalSelector,  &QSlider::valueChanged,
                     this, &StarProfileViewer::changeSelection);
    QObject::connect(selectorsVisible,  &QCheckBox::toggled,
                     horizontalSelector, &QWidget::setVisible);
    QObject::connect(selectorsVisible,  &QCheckBox::toggled,
                     verticalSelector, &QWidget::setVisible);
    QObject::connect(selectorsVisible,  &QCheckBox::toggled,
                     exploreMode, &QWidget::setVisible);
    QObject::connect(toggleEnableCutoff,  &QCheckBox::toggled,
                     this, &StarProfileViewer::toggleCutoffEnabled);
    QObject::connect(m_3DPixelSeries,  &QBar3DSeries::selectedBarChanged,
                     this, &StarProfileViewer::updateSelectorBars);

    m_graph->activeTheme()->setType(Q3DTheme::Theme(3)); //Stone Moss

    setGreenToRedGradient();

    m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetFront);
    m_graph->scene()->activeCamera()->setTarget(QVector3D(0.0f, 0.0f, 0.0f));
    m_graph->scene()->activeCamera()->setZoomLevel(110);

    show();
}

StarProfileViewer::~StarProfileViewer()
{
    delete m_graph;
}

void StarProfileViewer::loadData(FITSData * data, QRect sub, QList<Edge *> centers)
{
    if(data){
        imageData = data;
        subFrame=sub;
        starCenters=centers;

        // Create data arrays
        dataSet = new QBarDataArray;
        QBarDataRow *dataRow;
        dataSet->reserve(subFrame.height());
        QStringList rowLabels;
        QStringList columnLabels;

        for (int j = subFrame.y(); j < subFrame.y() + subFrame.height(); j++)
        {
            if( j % 10 == 0 )
                rowLabels << QString::number(j);
            else
                rowLabels << "";
            dataRow = new QBarDataRow(subFrame.width());
            int x = 0;
            for (int i = subFrame.x(); i < subFrame.x() + subFrame.width(); i++)
            {
                if( i % 10 == 0 )
                    columnLabels << QString::number(i);
                else
                    columnLabels << "";
                if( i > 0 && i < imageData->getWidth() && j > 0 && j < imageData->getHeight())
                    (*dataRow)[x].setValue(getImageDataValue(i, j));
                x++;
            }
            dataSet->insert(0, dataRow); //Note the row axis is displayed in the opposite direction of the y axis in the image.
        }
        std::reverse(rowLabels.begin(), rowLabels.end());

        m_3DPixelSeries->dataProxy()->setRowLabels(rowLabels);
        m_3DPixelSeries->dataProxy()->setColumnLabels(columnLabels);

        updateScale();

        // Add data to the data proxy (the data proxy assumes ownership of it)
        // We will retain a copy of the data set so that we can update the display
        updateDisplayData();

        updateHFRandPeakSelection();

        horizontalSelector->setRange(0, subFrame.width()-1);
        verticalSelector->setRange(0, subFrame.width()-1);  //Width and height are the same
    }
}

void StarProfileViewer::toggleCutoffEnabled(bool enable)
{
    cutoffSlider->setEnabled(enable);
    cutOffEnabled = enable;
    updateDisplayData();
}

void StarProfileViewer::updateScale()
{

    //We need to disconnect these so that changing their ranges doesn't affect things
    QObject::disconnect(blackPointSlider,  &QSlider::valueChanged,
                     this, &StarProfileViewer::updateVerticalAxis);
    QObject::disconnect(whitePointSlider,  &QSlider::valueChanged,
                     this, &StarProfileViewer::updateVerticalAxis);
    QObject::disconnect(cutoffSlider,  &QSlider::valueChanged,
                     this, &StarProfileViewer::updateDisplayData);

    float subFrameMin, subFrameMax;
    double dataMin, dataMax;
    float min, max;
    getSubFrameMinMax(&subFrameMin, &subFrameMax, &dataMin, &dataMax);

    int sliderDataMin = convertToSliderValue(dataMin);
    int sliderDataMax = convertToSliderValue(dataMax);

    if(autoScale->isChecked())
    {
        min = subFrameMin;
        max = subFrameMax;
        int sliderMin = convertToSliderValue(min);
        int sliderMax = convertToSliderValue(max);
        blackPointSlider->setRange(sliderMin, sliderMax);
        blackPointSlider->setTickInterval((sliderMax - sliderMin) / 100);
        whitePointSlider->setRange(sliderMin, sliderMax);
        whitePointSlider->setTickInterval((sliderMax - sliderMin) / 100);
        cutoffSlider->setRange(sliderMin, sliderDataMax);
        cutoffSlider->setTickInterval((sliderDataMax - sliderMin) / 100);
        blackPointSlider->setValue(sliderMin);
        whitePointSlider->setValue(sliderMax);
        cutoffSlider->setValue(sliderDataMax);
    }
    else
    {
        min = convertFromSliderValue(blackPointSlider->value());
        max = convertFromSliderValue(whitePointSlider->value());
        blackPointSlider->setRange(sliderDataMin, sliderDataMax);
        blackPointSlider->setTickInterval((sliderDataMax - sliderDataMin) / 100);
        whitePointSlider->setRange(sliderDataMin, sliderDataMax);
        whitePointSlider->setTickInterval((sliderDataMax - sliderDataMin) / 100);
        cutoffSlider->setRange(sliderDataMin, sliderDataMax);
        cutoffSlider->setTickInterval((sliderDataMax - sliderDataMin) / 100);

    }
    m_pixelValueAxis->setRange(min, max);

    if(cutOffEnabled)
        cutoffValue->setText("Cut: " + QString::number(convertFromSliderValue(cutoffSlider->value()), 'f', 2));
    else
        cutoffValue->setText("Cut Disabled");

    if(max < 10 )
    {
        m_pixelValueAxis->setLabelFormat(QString(QStringLiteral("%.3f ")));
        m_3DPixelSeries->setItemLabelFormat(QString(QStringLiteral("%.3f ")));
        maxValue->setText("Max: " + QString::number(max, 'f', 2));
        minValue->setText("Min: " + QString::number(min, 'f', 2));
    }
    else
    {
        m_pixelValueAxis->setLabelFormat(QString(QStringLiteral("%.0f ")));
        m_3DPixelSeries->setItemLabelFormat(QString(QStringLiteral("%.0f ")));
        maxValue->setText("Max: " + QString::number(max));
        minValue->setText("Min: " + QString::number(min));
    }

    QObject::connect(blackPointSlider,  &QSlider::valueChanged,
                     this, &StarProfileViewer::updateVerticalAxis);
    QObject::connect(whitePointSlider,  &QSlider::valueChanged,
                     this, &StarProfileViewer::updateVerticalAxis);
    QObject::connect(cutoffSlider,  &QSlider::valueChanged,
                     this, &StarProfileViewer::updateDisplayData);
}

void StarProfileViewer::zoomViewTo(int where)
{
    switch (where) {
    case 0: //Zoom To
        break;

    case 1: //Front
        m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetFront);
        m_graph->scene()->activeCamera()->setTarget(QVector3D(0.0f, 0.0f, 0.0f));
        m_graph->scene()->activeCamera()->setZoomLevel(110);
        zoomView->setCurrentIndex(0);
        break;

    case 2: //Front High
        m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetFrontHigh);
        m_graph->scene()->activeCamera()->setTarget(QVector3D(0.0f, 0.0f, 0.0f));
        m_graph->scene()->activeCamera()->setZoomLevel(110);
        zoomView->setCurrentIndex(0);
        break;

    case 3: //Overhead
        m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetDirectlyAbove);
        m_graph->scene()->activeCamera()->setTarget(QVector3D(0.0f, 0.0f, 0.0f));
        m_graph->scene()->activeCamera()->setZoomLevel(110);
        zoomView->setCurrentIndex(0);
        break;

    case 4: //Isometric L
        m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetIsometricLeftHigh);
        m_graph->scene()->activeCamera()->setTarget(QVector3D(0.0f, 0.0f, 0.0f));
        m_graph->scene()->activeCamera()->setZoomLevel(110);
        zoomView->setCurrentIndex(0);
        break;

    case 5: //Isometric R
        m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetIsometricRightHigh);
        m_graph->scene()->activeCamera()->setTarget(QVector3D(0.0f, 0.0f, 0.0f));
        m_graph->scene()->activeCamera()->setZoomLevel(110);
        zoomView->setCurrentIndex(0);
        break;

    case 6: //Selected Item
    {
        QPoint selectedBar = m_graph->selectedSeries()
                ? m_graph->selectedSeries()->selectedBar()
                : QBar3DSeries::invalidSelectionPosition();
        if (selectedBar != QBar3DSeries::invalidSelectionPosition())
        {
            QVector3D target;
            float xMin = m_graph->columnAxis()->min();
            float xRange = m_graph->columnAxis()->max() - xMin;
            float zMin = m_graph->rowAxis()->min();
            float zRange = m_graph->rowAxis()->max() - zMin;
            target.setX((selectedBar.y() - xMin) / xRange * 2.0f - 1.0f);
            target.setZ((selectedBar.x() - zMin) / zRange * 2.0f - 1.0f);

            qreal endAngleX = qAtan(qreal(target.z() / target.x())) / M_PI * -180.0 + 90.0;
            if (target.x() > 0.0f)
                endAngleX -= 180.0f;
            float barValue = m_graph->selectedSeries()->dataProxy()->itemAt(selectedBar.x(),
                                                                            selectedBar.y())->value();
            float endAngleY = 60.0f;
            float zoom = 150 * 1/qSqrt(barValue / convertFromSliderValue(whitePointSlider->value()));
            m_graph->scene()->activeCamera()->setCameraPosition(endAngleX, endAngleY, zoom);
            m_graph->scene()->activeCamera()->setTarget(target);


        }
        zoomView->setCurrentIndex(0);
        break;
    }


    default:
        break;
    }
}

void StarProfileViewer::changeSelectionType(int type)
{
    switch (type) {
    case 0:
        m_graph->setSelectionMode(QAbstract3DGraph::SelectionItem);
        m_graph->scene()->setSlicingActive(false);
        sliceB->setEnabled(false);
        break;

    case 1:
        m_graph->setSelectionMode(QAbstract3DGraph::SelectionItemAndRow);
        sliceB->setEnabled(true);
        break;

    case 2:
        m_graph->setSelectionMode(QAbstract3DGraph::SelectionItemAndColumn);
        sliceB->setEnabled(true);
        break;

    default:
        break;
    }
}

void StarProfileViewer::changeSelection()
{
    //Note row, column    y, x
    m_graph->primarySeries()->setSelectedBar(QPoint( verticalSelector->value(), horizontalSelector->value()));
    if(exploreMode->isChecked())
        zoomViewTo(6); //Zoom to SelectedItem
}

void StarProfileViewer::updateSelectorBars(QPoint position)
{
    //Note that we need to disconnect and then reconnect to avoid triggering changeSelection
    QObject::disconnect(verticalSelector,  &QSlider::valueChanged,
                     this, &StarProfileViewer::changeSelection);
    QObject::disconnect(horizontalSelector,  &QSlider::valueChanged,
                     this, &StarProfileViewer::changeSelection);

    //Note row, column    y, x
    verticalSelector->setValue(position.x());
    horizontalSelector->setValue(position.y());

    QObject::connect(verticalSelector,  &QSlider::valueChanged,
                     this, &StarProfileViewer::changeSelection);
    QObject::connect(horizontalSelector,  &QSlider::valueChanged,
                     this, &StarProfileViewer::changeSelection);
}

void StarProfileViewer::updateSampleSize(const QString &text)
{
    emit sampleSizeUpdated(text.toInt());
}

void StarProfileViewer::enableTrackingBox(bool enable)
{
    sampleSize->setVisible(enable);
}

void StarProfileViewer::updateDisplayData()
{
    if(cutOffEnabled)
        cutoffValue->setText("Cut: " + QString::number(convertFromSliderValue(cutoffSlider->value()), 'f', 2));
    else
        cutoffValue->setText("Cut Disabled");
    if(dataSet != nullptr)
    {
        QBarDataArray *displayDataSet = new QBarDataArray;
        displayDataSet->reserve(dataSet->size());

        for (int row = 0; row < dataSet->size(); row++)
        {
            QBarDataRow *dataRow = dataSet->at(row);
            QBarDataRow *newDataRow;
            newDataRow = new QBarDataRow(dataRow->size());
            for (int column = 0; column < dataRow->size(); column++)
            {
                if(cutOffEnabled && dataRow->value(column).value() > convertFromSliderValue(cutoffSlider->value()))
                    (*newDataRow)[column].setValue(0.0f);
                else
                    (*newDataRow)[column].setValue(dataRow->value(column).value());
            }
            displayDataSet->append(newDataRow);

        }
        m_3DPixelSeries->dataProxy()->resetArray(displayDataSet); //, m_3DPixelSeries->dataProxy()->rowLabels(), m_3DPixelSeries->dataProxy()->columnLabels()
    }
}

void StarProfileViewer::getSubFrameMinMax(float *subFrameMin, float *subFrameMax, double *dataMin, double *dataMax)
{
    imageData->getMinMax(dataMin,dataMax);

    //Backwards so that we can find the min and max in subFrame
    *subFrameMin = (float) *dataMax;
    *subFrameMax = (float) *dataMin;

    for (int y = subFrame.y(); y < subFrame.y() + subFrame.height(); y++)
    {
        for (int x = subFrame.x(); x < subFrame.x() + subFrame.width(); x++)
        {
            if( x > 0 && x < imageData->getWidth() && y > 0 && y < imageData->getHeight())
            {
                float value = getImageDataValue(x, y);
                if(value < *subFrameMin)
                    *subFrameMin = value;
                if(value > *subFrameMax)
                    *subFrameMax = value;
            }
        }
    }
}

template <typename T>
float StarProfileViewer::getImageDataValue(int x, int y)
{
    if(!imageData)
        return 0;
    uint8_t *image_buffer = imageData->getImageBuffer();
    T *buffer = reinterpret_cast<T *>(image_buffer);
    return (float) buffer[y * imageData->getWidth() + x];
}



float StarProfileViewer::getImageDataValue(int x, int y)
{
    switch (imageData->getDataType())
    {
        case TBYTE:
            return getImageDataValue<uint8_t>(x, y);
            break;

        case TSHORT:
            return getImageDataValue<int16_t>(x, y);
            break;

        case TUSHORT:
            return getImageDataValue<uint16_t>(x, y);
            break;

        case TLONG:
            return getImageDataValue<int32_t>(x, y);
            break;

        case TULONG:
            return getImageDataValue<uint32_t>(x, y);
            break;

        case TFLOAT:
            return getImageDataValue<float>(x, y);
            break;

        case TLONGLONG:
            return getImageDataValue<int64_t>(x, y);
            break;

        case TDOUBLE:
            return getImageDataValue<double>(x, y);
            break;

        default:
            return 0;
            break;
    }
}

void StarProfileViewer::toggleSlice()
{
    if(m_graph->selectionMode() == QAbstract3DGraph::SelectionItemAndRow || m_graph->selectionMode() == QAbstract3DGraph::SelectionItemAndColumn)
    {

        if(m_graph->scene()->isSlicingActive())
        {
            m_graph->scene()->setSlicingActive(false);
        }
        else
        {
            QPoint selectedBar = m_graph->selectedSeries()
                    ? m_graph->selectedSeries()->selectedBar()
                    : QBar3DSeries::invalidSelectionPosition();
            if (selectedBar != QBar3DSeries::invalidSelectionPosition())
                m_graph->scene()->setSlicingActive(true);
        }
    }
}

void StarProfileViewer::updateVerticalAxis()
{
    float blackPoint = convertFromSliderValue(blackPointSlider->value());
    float whitePoint = convertFromSliderValue(whitePointSlider->value());
    m_pixelValueAxis->setRange(blackPoint, whitePoint);
    maxValue->setText("Max: " + QString::number(whitePoint, 'f', 2));
    minValue->setText("Min: " + QString::number(blackPoint, 'f', 2));
}

void StarProfileViewer::updateHFRandPeakSelection()
{
    m_graph->removeCustomItems();

    HFRReportBox->setText("");
    peakValue->setText("");

    if(HFRReport->isChecked() || showPeakValues->isChecked())
    {

        for (int i = 0; i < starCenters.count(); i++)
        {
            int x = starCenters[i]->x;
            int row = x - subFrame.x();
            int y = starCenters[i]->y;
            int col = subFrame.height() - (y - subFrame.y());
            if(subFrame.contains(x,y)){
               double newHFR = imageData->getHFR(x,y);
               int value = getImageDataValue(x, y);
               QCustom3DLabel *label = new QCustom3DLabel();
               label->setFacingCamera(true);
               QString labelString = "";
               if(HFRReport->isChecked())
               {
                   labelString = labelString + "HFR: " + QString::number(newHFR, 'f', 2) + "  ";
                   HFRReportBox->setText(QString::number(newHFR, 'f', 2));
               }
               if(showPeakValues->isChecked())
               {
                   labelString = labelString + "Peak: " + QString::number(value);
                   peakValue->setText(QString::number(value));

               }
               label->setText(labelString);
               label->setPosition(QVector3D(row, value, col));
               label->setScaling(QVector3D(1.0f, 1.0f, 1.0f));
               m_graph->addCustomItem(label);
            }
        }
    }
}

void StarProfileViewer::updateColor(int selection)
{
    switch (selection) {
    case 0:
        setGreenToRedGradient();
        break;

    case 1:
        setBlackToYellowGradient();
        break;

    default:
        break;
    }
}

void StarProfileViewer::setBlackToYellowGradient()
{
    QLinearGradient gr;
    gr.setColorAt(0.0, Qt::black);
    gr.setColorAt(0.33, Qt::blue);
    gr.setColorAt(0.67, Qt::red);
    gr.setColorAt(1.0, Qt::yellow);

    QLinearGradient highGr;
    highGr.setColorAt(0.0, Qt::yellow);
    highGr.setColorAt(1.0, Qt::yellow);

    QLinearGradient sinHighGr;
    sinHighGr.setColorAt(0.0, Qt::red);
    sinHighGr.setColorAt(1.0, Qt::red);

    m_3DPixelSeries->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
    m_3DPixelSeries->setBaseGradient(gr);
    m_3DPixelSeries->setSingleHighlightGradient(sinHighGr);
    m_3DPixelSeries->setMultiHighlightGradient(highGr);
}

void StarProfileViewer::setGreenToRedGradient()
{
    QLinearGradient gr;
    gr.setColorAt(0.0, Qt::darkGreen);
    gr.setColorAt(0.5, Qt::yellow);
    gr.setColorAt(0.8, Qt::red);
    gr.setColorAt(1.0, Qt::darkRed);

    QLinearGradient highGr;
    highGr.setColorAt(0.0, Qt::black);
    highGr.setColorAt(1.0, Qt::black);

    QLinearGradient sinHighGr;
    sinHighGr.setColorAt(0.0, Qt::red);
    sinHighGr.setColorAt(1.0, Qt::red);

    m_3DPixelSeries->setBaseGradient(gr);
    m_3DPixelSeries->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
    m_3DPixelSeries->setSingleHighlightGradient(sinHighGr);
    m_3DPixelSeries->setMultiHighlightGradient(highGr);
}

//Multiplying by 1000000 will take care of preserving decimals in an int slider
//The sqrt function makes the slider non-linear, emphasising the lower values
//Note that it is actually multiplying the number on the slider by 1000 or so since it is square rooted.

int StarProfileViewer::convertToSliderValue(float value)
{
    return (int) qSqrt((value * 1000000.0));
}

float StarProfileViewer::convertFromSliderValue(int value)
{
    return qPow((float)value,2) / 1000000.0;
}
