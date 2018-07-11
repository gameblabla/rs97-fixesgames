/*
 * $Id: foecommand.h,v 1.2 2003/08/10 04:09:46 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Foe commands data.
 *
 * @version $Revision: 1.2 $
 */
#ifndef FOECOMMAND_H_
#define FOECOMMAND_H_

#include "bulletmlparser.h"
#include "bulletmlparser-tinyxml.h"
#include "bulletmlrunner.h"
#include "foe.h"

class FoeCommand : public BulletMLRunner {
 public:
  FoeCommand(BulletMLParser* parser, struct foe* f);
  FoeCommand(BulletMLState* state, struct foe* f);

  virtual ~FoeCommand();

  virtual double getBulletDirection();
  virtual double getAimDirection();
  virtual double getBulletSpeed();
  virtual double getDefaultSpeed();
  virtual double getRank();
  virtual void createSimpleBullet(double direction, double speed);
  virtual void createBullet(BulletMLState* state, double direction, double speed);
  virtual int getTurn();
  virtual void doVanish();
  
  virtual void doChangeDirection(double d);
  virtual void doChangeSpeed(double s);
  virtual void doAccelX(double ax);
  virtual void doAccelY(double ay);
  virtual double getBulletSpeedX();
  virtual double getBulletSpeedY();
  
 private:
  struct foe *foe;
};
#endif


