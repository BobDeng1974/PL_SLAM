/*****************************************************************************
**      Stereo VO and SLAM by combining point and line segment features     **
******************************************************************************
**                                                                          **
**  Copyright(c) 2016-2018, Ruben Gomez-Ojeda, University of Malaga         **
**  Copyright(c) 2016-2018, David Zuñiga-Noël, University of Malaga         **
**  Copyright(c) 2016-2018, MAPIR group, University of Malaga               **
**                                                                          **
**  This program is free software: you can redistribute it and/or modify    **
**  it under the terms of the GNU General Public License (version 3) as     **
**  published by the Free Software Foundation.                              **
**                                                                          **
**  This program is distributed in the hope that it will be useful, but     **
**  WITHOUT ANY WARRANTY; without even the implied warranty of              **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            **
**  GNU General Public License for more details.                            **
**                                                                          **
**  You should have received a copy of the GNU General Public License       **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.   **
**                                                                          **
*****************************************************************************/

#include <stereoFeatures.h>
#include "auxiliar.h"
#include "config.h"
namespace StVO{


PointFeature::PointFeature( Vector3d P_, Vector2d pl_obs_) :
    P(P_), pl_obs(pl_obs_), level(0)
{}

PointFeature::PointFeature( Vector2d pl_, double disp_, Vector3d P_, int idx_ ) :
    pl(pl_), disp(disp_), P(P_), inlier(true), idx(idx_), level(0)
{}

PointFeature::PointFeature( Vector2d pl_, double disp_, Vector3d P_, int idx_, int level_ ) :
    pl(pl_), disp(disp_), P(P_), inlier(true), idx(idx_), level(level_)
{
    for( int i = 0; i < level; i++ )
        sigma2 *= Config::orbScaleFactor();
    sigma2 = 1.f / (sigma2*sigma2);
}

PointFeature::PointFeature( Vector2d pl_, double disp_, Vector3d P_, int idx_, int level_, Matrix3d covP_an_ ) :
    pl(pl_), disp(disp_), P(P_), inlier(true), idx(idx_), level(level_), covP_an(covP_an_)
{
    for( int i = 0; i < level; i++ )
        sigma2 *= Config::orbScaleFactor();
    sigma2 = 1.f / (sigma2*sigma2);
}

PointFeature::PointFeature( Vector2d pl_, double disp_, Vector3d P_, Vector2d pl_obs_,
              int idx_, int level_, double sigma2_, Matrix3d covP_an_, bool inlier_ , Vector3d P_obs) :
    pl(pl_), disp(disp_), P(P_), pl_obs(pl_obs_), inlier(inlier_), level(level_), sigma2(sigma2_), covP_an(covP_an_), P_obs(P_obs)
{}

PointFeature* PointFeature::safeCopy(){
    return new PointFeature( pl, disp, P, pl_obs, idx, level, sigma2, covP_an, inlier, P_obs );
}



// Line segment feature



LineFeature::LineFeature( Vector3d sP_, Vector3d eP_, Vector3d le_obs_, Vector2d spl_obs_, Vector2d epl_obs_) :
    sP(sP_), eP(eP_), le_obs(le_obs_), spl_obs(spl_obs_), epl_obs(epl_obs_), level(0)
{
    plucCam.head(3)=skew(sP_)*eP_;
    plucCam.tail(3)=eP_-sP_;
}



LineFeature::LineFeature( Vector2d spl_, double sdisp_, Vector3d sP_,
                          Vector2d epl_, double edisp_, Vector3d eP_,
                          Vector3d le_,  double angle_, int    idx_) :
    spl(spl_), sdisp(sdisp_), sP(sP_), epl(epl_), edisp(edisp_), eP(eP_), le(le_), inlier(true), idx(idx_), angle(angle_), level(0)
{
    plucCam.head(3)=skew(sP_)*eP_;
    plucCam.tail(3)=eP_-sP_;
}

LineFeature::LineFeature( Vector2d spl_, double sdisp_, Vector3d sP_,
                          Vector2d epl_, double edisp_, Vector3d eP_,
                          Vector3d le_,  double angle_, int idx_, int level_) :
    spl(spl_), sdisp(sdisp_), sP(sP_), epl(epl_), edisp(edisp_), eP(eP_), le(le_), inlier(true), idx(idx_), angle(angle_), level(level_)
{
    plucCam.head(3)=skew(sP_)*eP_;
    plucCam.tail(3)=eP_-sP_;
    for( int i = 0; i < level; i++ )
        sigma2 *= Config::lsdScale();
    sigma2 = 1.f / (sigma2*sigma2);
}

LineFeature::LineFeature( Vector2d spl_, double sdisp_, Vector3d sP_, Vector2d spl_obs_, double sdisp_obs_,
                          Vector2d epl_, double edisp_, Vector3d eP_, Vector2d epl_obs_, double edisp_obs_,
                          Vector3d le_, Vector3d le_obs_, double angle_, int idx_, int level_, bool inlier_, double sigma2_,
                          Matrix3d covEpt3D_, Matrix3d covSpt3D_, Plucker plucCam_obs_) :

    spl(spl_), sdisp(sdisp_), sP(sP_), spl_obs(spl_obs_), sdisp_obs(sdisp_obs_),
    epl(epl_), edisp(edisp_), eP(eP_), epl_obs(epl_obs_), edisp_obs(edisp_obs_),
    le(le_), le_obs(le_obs_), angle(angle_), idx(idx_), level(level_), inlier(inlier_), sigma2(sigma2_), covEpt3D(covEpt3D_), covSpt3D(covSpt3D_),
    plucCam_obs(plucCam_obs_)
{
    plucCam.head(3)=skew(sP_)*eP_;
    plucCam.tail(3)=eP_-sP_;
    for( int i = 0; i < level; i++ )
        sigma2 *= Config::lsdScale();
    sigma2 = 1.f / (sigma2*sigma2);
}

LineFeature* LineFeature::safeCopy(){
    return new LineFeature( spl, sdisp, sP, spl_obs, sdisp_obs,
                            epl, edisp, eP, epl_obs, edisp_obs,
                            le, le_obs, angle, idx, level, inlier, sigma2, covEpt3D, covSpt3D,plucCam_obs);
}



void LineFeature::getCov3DStereo(PinholeStereoCamera* cam)
{
    //得到像素坐标的不确定性
    double spl_std = 1.0;  // 2.0; //
    double epl_std = 1.0;  // 2.0; //
    // fit the coefficient with synthetic data 得到视差不确定性
    double sdisp_std, edisp_std;
    //判断是否水平
    if (fabs(le(0)) > 0.15) {
        //ratioDispSTD 0.15 不确定性和视差成正比
        sdisp_std = Config::ratioDispSTD() * sdisp;
        edisp_std = Config::ratioDispSTD() * edisp;
        //sdisp=this->cam->getFx() * this->cam->getB() / sP(2)视差的计算公式
    }
    else {
        //水平方向上的比率就大一点 ratioDispSTDHor 0.9
        sdisp_std = Config::ratioDispSTDHor() * sdisp;
        edisp_std = Config::ratioDispSTDHor() * edisp;
    }
        
    //定义像素误差 
    //求起始点相机坐标系协方差
    Matrix3d cov2D=Matrix3d::Zero();
    cov2D(0, 0) = pow( spl_std, 2 );
    cov2D(1, 1) = pow( epl_std, 2 );
    cov2D(2, 2) = pow( sdisp_std, 2 );
    //雅克比矩阵是相机坐标系下的XYZ对u，v，d的雅克比矩阵
    Matrix3d jacMat;
    getStereoJacob3D_2D(spl(0),spl(1),sdisp,cam->getB(),cam->getFx(),cam->getCx(),cam->getCy(),jacMat);
    //求得相机坐标系的坐标协方差矩阵
    covSpt3D = jacMat * cov2D * jacMat.transpose();
    
    //求终止点相机坐标系协方差
    cov2D(0, 0) = pow( spl_std, 2 );
    cov2D(1, 1) = pow( epl_std, 2 );
    cov2D(2, 2) = pow( sdisp_std, 2 );
    getStereoJacob3D_2D(epl(0),epl(1),edisp,cam->getB(),cam->getFx(),cam->getCx(),cam->getCy(),jacMat);
    //求得相机坐标系的坐标协方差矩阵
    covEpt3D = jacMat * cov2D * jacMat.transpose();
}

}
