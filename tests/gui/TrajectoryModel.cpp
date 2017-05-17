#include "TrajectoryModel.hpp"
#include <iostream>

TrajectoryModel::TrajectoryModel(const std::string servoName, Trajectory& trajectory) :
    QAbstractTableModel(nullptr),
    trajectory(trajectory), colCount(1), servoName(servoName)
{
    initTrajectory();
}


void TrajectoryModel::initTrajectory()
{
    trajectory.size = 1;
    for(int i = 0; i < MAX_TRAJECTORY_SIZE; ++i)
    {
        trajectory.data[i].angle = 0;
        trajectory.data[i].time = 0;
    }
}

int TrajectoryModel::columnCount(const QModelIndex& parent) const
{
    return MAX_TRAJECTORY_SIZE;
}

QVariant TrajectoryModel::data(const QModelIndex& index, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();
    
    const int row = index.row();
    const int col = index.column();
    
    if(row >= 2)
    {
        std::cerr << "TrajectoryModel: row index out of range: " << row << std::endl;
        return QVariant();
    }
    
    if(col >= MAX_TRAJECTORY_SIZE)
    {
        std::cerr << "TrajectoryModel: column index out of range: " << col;
        return QVariant();
    }
    switch(row)
    {
        case 0:
            return QVariant(trajectory.data[col].time);
        case 1:
            return QVariant(trajectory.data[col].angle);
        default:
            std::cerr << "TrajectoryModel: row index out of range: " << row << std::endl;
    }
    
    return QVariant();
}

QVariant TrajectoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    
    if(orientation == Qt::Vertical) 
    {
        switch(section)
        {
            case 0:
                return tr("Time");
            case 1:
                return tr("Angle");
        }
    }
    else if(orientation == Qt::Horizontal)
        return QString::number(section);
    
  return QVariant();
}

int TrajectoryModel::rowCount(const QModelIndex& parent) const
{
    return 2;
}


bool TrajectoryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::EditRole)
    {
        const int row = index.row();
        const int col = index.column();
        
        if(row >= 2)
        {
            std::cerr << "TrajectoryModel: row index out of range: " << row << std::endl;
            return false;
        }
        
        if(col >= MAX_TRAJECTORY_SIZE)
        {
            std::cerr << "TrajectoryModel: column index out of range: " << col<< std::endl;
            return false;
        }
        
        bool convertOk = false;
        const int intValue = value.toInt(&convertOk);
        
        if(!convertOk)
        {
            std::cerr << "TrajectoryModel: Convert to int failed" << std::endl;
            return false;
        }
        
        switch(row)
        {
            case 0:
                trajectory.data[col].time = intValue;
                break;
            case 1:
                trajectory.data[col].angle = intValue;
                break;
            default:
                std::cerr << "TrajectoryModel: row index out of range: " << col;
                    return false;
        }
    }
    
    emit trajectoryChanged(servoName, trajectory);
    
    return true;
}

Qt::ItemFlags TrajectoryModel::flags(const QModelIndex& index) const
{
    if(index.column() < colCount)
        return  Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    else 
        return Qt::NoItemFlags;
}


void TrajectoryModel::setSize(int size)
{
    if(size <= 0)
        size = 1;
    if(size > MAX_TRAJECTORY_SIZE)
        size = MAX_TRAJECTORY_SIZE;
    
    colCount = size;
    trajectory.size = size;
    //refresh everything, inefficient but who cares :)
    const QModelIndex topLeft = index(0, 0);
    const QModelIndex bottomRight = index(1, MAX_TRAJECTORY_SIZE);
    emit dataChanged(topLeft, bottomRight);
    emit trajectoryChanged(servoName, trajectory);
}

void TrajectoryModel::trajectoryUpdated()
{
    const QModelIndex topLeft = index(0, 0);
    const QModelIndex bottomRight = index(1, MAX_TRAJECTORY_SIZE);
    emit dataChanged(topLeft, bottomRight);
    emit trajectoryChanged(servoName, trajectory);
}









