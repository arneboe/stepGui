#pragma once
#include <QAbstractTableModel>
#include "../../engine.hpp"


class TrajectoryModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TrajectoryModel(const std::string servoName, Trajectory& trajectory);
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    
    /**set trajectory size */
    void setSize(int size);
    
public slots:
    /**Tell the model that the trajectory has changed */
    void trajectoryUpdated();
    
signals:
    void trajectoryChanged(const std::string& servoName, const Trajectory& traj);
        
    
private:
    
    void initTrajectory();
    Trajectory& trajectory;
    int colCount;
    std::string servoName;
};
