//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//

#ifndef CalorimeterSD_h
#define CalorimeterSD_h 1

#include "G4VSensitiveDetector.hh"
#include "CaloHit.hh"
#include "CalorimeterSDMessenger.hh"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

class CalorimeterSD : public G4VSensitiveDetector
{
  
public:
  CalorimeterSD(G4String name);
  ~CalorimeterSD();
  
  void Initialize(G4HCofThisEvent*HCE);
  G4int GetDetID(G4Step*aStep);
  G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
  void EndOfEvent(G4HCofThisEvent*HCE);
  void clear();
  void DrawAll();
  void PrintAll();

  inline void SetUseBirksLaw(G4bool aVal){useBirks=aVal;}

private:
  G4double BirksAttenuation(const G4Step*);

  CalorimeterSDMessenger* fMessenger;
  CaloHitsCollection* CaloCollection;
  G4int verboseLevel;
  std::map<int,int> LayerID;
  G4bool useBirks;

  G4double birk1scint;
  G4double birk2scint;
  G4double birk3scint;

  G4double birk1crystal;
  G4double birk2crystal;
  G4double birk3crystal;
};




#endif

