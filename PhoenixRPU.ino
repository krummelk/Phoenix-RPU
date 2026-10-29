/**************************************************************************
    PhoenixRPU - custom rules for Williams Phoenix (1978)
    VERSION: v21 (SKETCH_VERSION 21, shown on the credits display at power-up)
    NEW IN THIS VERSION (vs v20): from ball 2 on, the drop target banks (and anything else fired without the
    "override" flag) were never reset mid-ball: the bonus countdown at the end of each ball switches the solenoid
    stack off, and only the start of a GAME switched it back on. Each new ball now switches it on again.
    (Still from v20: the first-hit rule is ball 1 only, spinner counts. From v19: inlanes light "2" and "4".)
    Change history: see the PhoenixRPU-CHANGELOG text file delivered with each version.

    This pinball code is distributed in the hope that it
    will be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    See <https://www.gnu.org/licenses/>.
*/

#include "RPU_Config.h"
#include "RPU.h"
#include "DropTargets.h"
#include "PhoenixRPU.h"
#include "OperatorMenus.h"
#include "AudioHandler.h"
#include "DisplayHandler.h"
#include "LampAnimations.h"
#ifdef RPU_OS_USE_ACCESSORY_LAMP_BOARD
#include "ALB-Communication.h"
#endif
#include <EEPROM.h>


#define GAME_MAJOR_VERSION  2026
#define GAME_MINOR_VERSION  1
#define DEBUG_MESSAGES  1

#if (DEBUG_MESSAGES==1)
//#define DEBUG_SHOW_LOOPS_PER_SECOND
#endif

#ifdef RPU_SIMPLIFY_DISPLAY_FOR_7VOLUTION
#define DISPLAY_DASH_STYLE  DISPLAY_DASH_INTERMITTENT_FLASH
#else
#define DISPLAY_DASH_STYLE  DISPLAY_DASH_ROLLING_BLANK
#endif


/*********************************************************************

    Game specific code

*********************************************************************/

// MachineState
//  0 - Attract Mode
//  negative - no longer used
//  positive - game play
boolean InOperatorMenu = false;
char MachineState = 0;
boolean MachineStateChanged = true;
#define MACHINE_STATE_ATTRACT         0
#define MACHINE_STATE_INIT_GAMEPLAY   1
#define MACHINE_STATE_INIT_NEW_BALL   2
#define MACHINE_STATE_NORMAL_GAMEPLAY 4
#define MACHINE_STATE_COUNTDOWN_BONUS 99
#define MACHINE_STATE_BALL_OVER       100
#define MACHINE_STATE_MATCH_MODE      110
#define MACHINE_STATE_DIAGNOSTICS     120

// Indices of EEPROM save locations
#define EEPROM_RPOS_INIT_PROOF_UL                 90

// CUSTOMIZATION TODO:
// This value needs to be set to a UNIQUE value for the 
// game code. You can pick any number, or do like I did and 
// ASCII encode some text (this value translates to "EM01")
#define RPOS_INIT_PROOF                           0x454D3031

#define EEPROM_BALL_SAVE_BYTE                     100
#define EEPROM_FREE_PLAY_BYTE                     101
#define EEPROM_TILT_WARNING_BYTE                  104
#define EEPROM_AWARD_OVERRIDE_BYTE                105
#define EEPROM_BALLS_OVERRIDE_BYTE                106
#define EEPROM_TOURNAMENT_SCORING_BYTE            107
#define EEPROM_SFX_VOLUME_BYTE                    108
#define EEPROM_MUSIC_VOLUME_BYTE                  109
#define EEPROM_SCROLLING_SCORES_BYTE              110
#define EEPROM_CALLOUTS_VOLUME_BYTE               111
#define EEPROM_CRB_HOLD_TIME                      118
#define EEPROM_TROUGH_EJECT_STRENGTH              131
#define EEPROM_SAUCER_EJECT_STRENGTH              134
#define EEPROM_SLINGSHOT_STRENGTH                 135 
#define EEPROM_POP_BUMPER_STRENGTH                136
#define EEPROM_GAME_RULES_SELECTION               137
#define EEPROM_MATCH_FEATURE_BYTE                 138
#define EEPROM_EXTRA_BALL_SCORE_UL                160
#define EEPROM_SPECIAL_SCORE_UL                   164


#define GAME_MODE_SKILL_SHOT                        1
#define GAME_MODE_UNSTRUCTURED_PLAY                 2
#define GAME_MODE_EXAMPLE_1                         3

#define SOUND_EFFECT_NONE                     0
#define SOUND_EFFECT_SPINNER                  2
#define SOUND_EFFECT_TILT                     3
#define SOUND_EFFECT_TILT_WARNING             4
#define SOUND_EFFECT_SCORE_TICK               5
#define SOUND_EFFECT_POP_BUMPER               6
#define SOUND_EFFECT_LEFT_SLING               7
#define SOUND_EFFECT_RIGHT_SLING              8
#define SOUND_EFFECT_GAME_OVER                21
#define SOUND_EFFECT_MATCH_SPIN               28
#define SOUND_EFFECT_DROP_TARGET_SOUND_1      30
#define SOUND_EFFECT_DROP_TARGET_COMPLETE     40
#define SOUND_EFFECT_BONUS_1                  81
#define SOUND_EFFECT_BONUS_2                  82
#define SOUND_EFFECT_BONUS_3                  83
#define SOUND_EFFECT_BONUS_4                  84
#define SOUND_EFFECT_BONUS_5                  85
#define SOUND_EFFECT_BONUS_6                  86
#define SOUND_EFFECT_BONUS_7                  87
#define SOUND_EFFECT_BONUS_8                  88
#define SOUND_EFFECT_STARTUP_1                100
#define SOUND_EFFECT_STARTUP_2                101

#define SOUND_EFFECT_COIN_DROP_1              105
#define SOUND_EFFECT_COIN_DROP_2              106
#define SOUND_EFFECT_COIN_DROP_3              107

#define SOUND_EFFECT_BACKGROUND_SONG_1                400
#define SOUND_EFFECT_BACKGROUND_SONG_2                401
#define SOUND_EFFECT_BACKGROUND_SONG_3                402
#define SOUND_EFFECT_BACKGROUND_SONG_4                403
#define SOUND_EFFECT_BACKGROUND_SONG_5                404
#define SOUND_EFFECT_BACKGROUND_SONG_6                405

#define SOUND_EFFECT_VP_GAME_RULES_EASY               423
#define SOUND_EFFECT_VP_GAME_RULES_MEDIUM             424
#define SOUND_EFFECT_VP_GAME_RULES_HARD               425

#define SOUND_EFFECT_RALLY_MUSIC_1                    450
#define SOUND_EFFECT_RALLY_MUSIC_2                    451
#define SOUND_EFFECT_RALLY_MUSIC_3                    452
#define SOUND_EFFECT_RALLY_MUSIC_4                    453
#define SOUND_EFFECT_RALLY_MUSIC_5                    454


// Game play status callouts
#define SOUND_EFFECT_VP_PLAYER_1_UP                   301
#define SOUND_EFFECT_VP_PLAYER_2_UP                   302
#define SOUND_EFFECT_VP_PLAYER_3_UP                   303
#define SOUND_EFFECT_VP_PLAYER_4_UP                   304
#define SOUND_EFFECT_VP_EXTRA_BALL                    305

#define SOUND_EFFECT_VP_ADD_PLAYER_1        306
#define SOUND_EFFECT_VP_ADD_PLAYER_2        (SOUND_EFFECT_VP_ADD_PLAYER_1+1)
#define SOUND_EFFECT_VP_ADD_PLAYER_3        (SOUND_EFFECT_VP_ADD_PLAYER_1+2)
#define SOUND_EFFECT_VP_ADD_PLAYER_4        (SOUND_EFFECT_VP_ADD_PLAYER_1+3)
#define SOUND_EFFECT_VP_SHOOT_AGAIN         310

#define SOUND_EFFECT_VP_BALL_SAVE                       326

#define SOUND_EFFECT_DIAG_START                   1900
#define SOUND_EFFECT_DIAG_CREDIT_RESET_BUTTON     1900
#define SOUND_EFFECT_DIAG_SELECTOR_SWITCH_ON      1901
#define SOUND_EFFECT_DIAG_SELECTOR_SWITCH_OFF     1902
#define SOUND_EFFECT_DIAG_STARTING_ORIGINAL_CODE  1903
#define SOUND_EFFECT_DIAG_STARTING_NEW_CODE       1904
#define SOUND_EFFECT_DIAG_ORIGINAL_CPU_DETECTED   1905
#define SOUND_EFFECT_DIAG_ORIGINAL_CPU_RUNNING    1906
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_U10         1907
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_U11         1908
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_1           1909
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_2           1910
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_3           1911
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_4           1912
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_5           1913
#define SOUND_EFFECT_DIAG_STARTING_DIAGNOSTICS    1914

// CUSTOMIZATION TODO:
// Decide how much bonus you want to be able to display (based on the lamps you
// have available)
#define MAX_DISPLAY_BONUS     29   // 20K lamp + 9K lamp
#define TILT_WARNING_DEBOUNCE_TIME        1000
#define BALL_SAVE_GRACE_PERIOD            3000

/*********************************************************************

    Machine state and options

*********************************************************************/
byte Credits = 0;
byte BallSaveNumSeconds = 0;
byte MaximumCredits = 8;           // credits stop at 8
byte BallsPerGame = 3;
byte ScoreAwardReplay = 0;
byte MusicVolume = 6;
byte SoundEffectsVolume = 8;
byte CalloutsVolume = 10;
byte ChuteCoinsInProgress[3];
byte TotalBallsLoaded = 1;
byte TimeRequiredToResetGame = 1;
boolean FreePlayMode = false;
boolean MatchEnabled = true;
boolean HighScoreReplay = true;
boolean MatchFeature = true;
boolean TournamentScoring = false;
boolean ScrollingScores = true;
boolean GIInOriginal = true;
unsigned long ExtraBallValue = 0;
unsigned long SpecialValue = 0;
unsigned long CurrentTime = 0;
unsigned long HighScore = 0;
unsigned long AwardScores[3];
unsigned long CreditResetPressStarted = 0;
unsigned long OperatorSwitchPressStarted = 0;

#ifdef RPU_OS_USE_ACCESSORY_LAMP_BOARD
unsigned long ALBWatchdogResetLastSent;
#endif

#define NUM_CPC_PAIRS 9
boolean CPCSelectionsHaveBeenRead = false;
byte CPCPairs[NUM_CPC_PAIRS][2] = {
  {1, 5},
  {1, 4},
  {1, 3},
  {1, 2},
  {1, 1},
  {2, 3},
  {2, 1},
  {3, 1},
  {4, 1}
};
byte CPCSelection[3];

AudioHandler  Audio;
OperatorMenus Menus;

#ifdef RPU_OS_USE_ACCESSORY_LAMP_BOARD
AccessoryLampBoard TopperALB;
#endif


/*********************************************************************

    Game State

*********************************************************************/
byte CurrentPlayer = 0;
byte CurrentBallInPlay = 1;
byte CurrentNumPlayers = 0;
byte NumberOfBallsLocked;
byte NumberOfBallsInPlay;
byte Bonus[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte BonusX[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte GameMode = GAME_MODE_SKILL_SHOT;
byte LastGameMode = 0;
byte MaxTiltWarnings = 2;
byte NumTiltWarnings = 0;
byte CurrentAchievements[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte NumberOfBallSavesRemaining;

boolean SamePlayerShootsAgain = false;
boolean BallSaveUsed = false;
boolean SpecialAvailable = false;
boolean SpecialCollected = false;
boolean TimersPaused = true;

unsigned long CurrentScores[RPU_NUMBER_OF_PLAYERS_ALLOWED];
unsigned long BallFirstSwitchHitTime = 0;
unsigned long BallTimeInTrough = 0;
unsigned long GameModeStartTime = 0;
unsigned long GameModeEndTime = 0;
unsigned long LastTiltWarningTime;
unsigned long PlayfieldMultiplier;
unsigned long LastTimeThroughLoop;
unsigned long LastSwitchHitTime;
unsigned long BallSaveEndTime;


/*********************************************************************

    Game Specific State Variables

*********************************************************************/
byte ExtraBallAvailable[RPU_NUMBER_OF_PLAYERS_ALLOWED];   // 0 = not earned, 1 = lit (earned, waiting to be collected)

// Banked extra balls: a player can hold up to MAX_EXTRA_BALLS at once.
// One is spent each time a ball ends. Shoot Again lamps blink once per banked
// ball (SHOOT_AGAIN_BLINK_ON_MS lit, one blink every SHOOT_AGAIN_BLINK_SLOT_MS),
// then stay dark for SHOOT_AGAIN_PAUSE_MS before the pattern repeats.
#define MAX_EXTRA_BALLS               5
#define SHOOT_AGAIN_BLINK_ON_MS       500
#define SHOOT_AGAIN_BLINK_SLOT_MS     1000
#define SHOOT_AGAIN_PAUSE_MS          3000
byte ExtraBallsBanked[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte ExtraBallsAwarded[RPU_NUMBER_OF_PLAYERS_ALLOWED];   // extra balls earned so far this game (limit MAX_EXTRA_BALLS per player per game)
unsigned long ShootAgainCycleStart = 0;

// P-H-O-E-N-I-X progress (handbook p.5 and p.10). All of this is kept per player from ball to ball.
//   Completing the five "1"-"5" rollover lamps spots letters in order and clears the five lamps again.
//   "Normal" play (factory setting) spots P and H together on the first completion.
//   Letter E: lights Extra Ball When Lit, so the bullseye (switch 21) awards an extra ball.
//   Letter I: the outside rollovers alternate for a special, configured to award an extra ball.
//   Letter X: the bullseye awards the special (an extra ball) and turns X off; completing 1-5 turns it back on.
#define PHOENIX_LETTER_E              4       // PhoenixLetters count when E has just been lit (P,H,O,E)
#define PHOENIX_LETTER_I              6       // count when I has just been lit (P,H,O,E,N,I)
#define PHOENIX_PLAY_CONSERVATIVE     true    // (no longer used) P and H are always spotted separately, one letter per 1-5
#define OUTLANE_SPECIAL_ALTERNATE_MS  1000    // how often the lit outside rollover swaps sides
// Scores wrap at 999,999 (6-digit display). MillionsCount[] remembers how many times each
// player has wrapped. Once a player is over a million their display scrolls the whole score
// (for example 1,234,560) from right to left, rests on the normal 6 digits for 10 seconds and scrolls again.
#define SCORE_WRAP_VALUE              1000000
#define MILLIONS_SCROLL_STEP_MS       180     // time each digit takes to step across the display
#define MILLIONS_PAUSE_MS             10000   // normal 6-digit score shown between scrolls
byte MillionsCount[RPU_NUMBER_OF_PLAYERS_ALLOWED];
boolean MillionsBlinkDark[RPU_NUMBER_OF_PLAYERS_ALLOWED];          // true while that display is mid-scroll
unsigned long MillionsScrollStart[RPU_NUMBER_OF_PLAYERS_ALLOWED];  // when the current 10-second cycle began
byte PhoenixLetters[RPU_NUMBER_OF_PLAYERS_ALLOWED];           // letters P,H,O,E,N,I spotted so far (0-6)
byte PhoenixCompletions[RPU_NUMBER_OF_PLAYERS_ALLOWED];       // times P-H-O-E-N-I-X has been completed and started over
unsigned long PhoenixCompleteTime[RPU_NUMBER_OF_PLAYERS_ALLOWED]; // when it was last completed (starts the blink clock)
#define PHOENIX_BLINK_INTERVAL_MS     30000   // completed PHOENIX blinks every 30 seconds
#define PHOENIX_BLINK_PERIOD_MS       500     // one blink = all lamps on 250 ms, then off 250 ms
byte PhoenixSets[RPU_NUMBER_OF_PLAYERS_ALLOWED];              // how many times 1-5 has been completed
byte RolloversLit[RPU_NUMBER_OF_PLAYERS_ALLOWED];             // bits 0-4 = "1" through "5"
boolean PhoenixXLit[RPU_NUMBER_OF_PLAYERS_ALLOWED];           // bullseye special is armed
boolean OutlaneSpecialReady[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte PhoenixLetterLamps[6] = {LAMP_P, LAMP_H, LAMP_O, LAMP_E, LAMP_N, LAMP_I};
byte RolloverLamps[5] = {LAMP_1, LAMP_2, LAMP_3, LAMP_4, LAMP_5};

// Scoring from the handbook (p.5). "All other scoring is as indicated on the playfield",
// so the values below that are 0 are placeholders until they are confirmed.
#define SCORE_JET_SIDE                10      // left and right jet bumpers
#define SCORE_JET_CENTER              100
#define SCORE_KICKER                  10      // left and right kickers
#define SCORE_BANK_STANDUP            10      // standups on the drop target bank assemblies (22 and 24)
#define SCORE_STANDUP                 50      // left standup, top right, bottom left, bottom right
#define SCORE_BULLSEYE                5000
#define SCORE_SPINNER                 100
#define SCORE_SPINNER_LIT             1000    // spinner lit while bonus is 10,000 to 19,000
#define SCORE_NUMBER_ROLLOVER         1000    // "1"-"5" rollovers: 1000 and advance bonus (playfield)
#define SCORE_INSIDE_ROLLOVER         1000    // 1000 and advance bonus (playfield)
#define SCORE_OUTSIDE_ROLLOVER        1000    // 1000 and advance bonus (playfield)
#define SCORE_CENTER_DROP             10000   // A and B centre drop targets: 10,000 and advance bonus (playfield)
#define SCORE_SIDE_TARGET             100     // targets 14 and 19 (light "3"): nothing printed, recommended value
#define SCORE_DROP_TARGET             1000    // single bank drop target: 1,000 (measured on the stock game)
#define SCORE_DROP_BANK_COMPLETE      9000    // extra for the 4th target, so completing a bank scores 10,000 in all (measured on the stock game); also advances the multiplier and the bonus
// Bullseye: 5000 and advance bonus, or the highest lit letter value if that is higher
// P 10,000  H 10,000  O 20,000  E 20,000  N 30,000  I 30,000  X 40,000 (playfield)
// Collecting the lit extra ball with the bullseye also scores this much on top of the letter value
// (stock game: bullseye with P-H-O-E lit and extra ball lit scored 60,000 = 20,000 letter + 40,000)
#define SCORE_BULLSEYE_EXTRA_BALL     40000
const unsigned long PhoenixLetterValues[7] = {10000, 10000, 20000, 20000, 30000, 30000, 40000};
byte GameRulesSelection;
// CUSTOMIZATION TODO:
// if a solenoid is going to have adjustable strength, create variables for them
byte BallServeSolenoidStrength;  
byte SaucerSolenoidStrength;

int PopBumperProgress;
int SpinnerProgress[RPU_NUMBER_OF_PLAYERS_ALLOWED];

#define GAME_RULES_EASY         1
#define GAME_RULES_MEDIUM       2
#define GAME_RULES_HARD         3
#define GAME_RULES_PROGRESSIVE  4
#define GAME_RULES_CUSTOM       5

unsigned long PlayfieldMultiplierTimeLeft;
unsigned long BonusChanged;
unsigned long BonusXAnimationStart;
unsigned long LastTimePopHit;
unsigned long LastTimeBallServed;
unsigned long LastSpinnerHitTime;
unsigned long LastTimeSaucerSeen;

#define DROP_TARGET_RESET_STRENGTH    50   // ms; was 10, then 30 - still too weak to latch the targets up. Raise again if they don't lock
// On the System Alpha (rev 200) the "strength" numbers above are ignored: the board's own firmware times every coil pulse.
// The real pulse length is sent to the board once at power-up with RPU_SetSolenoidDefaultPulse() (see setup()).
#define DROP_TARGET_RESET_PULSE_MS    60   // ms for each of the six drop-target reset coils; raise it (try 80, 100) if targets don't latch up
#define DROP_BANK_RESET_STAGGER_MS    300  // the right bank resets this long after the left, so the big coils don't all fire together
#define DROP_BANK_COMPLETE_RESET_DELAY_MS 100  // 4th target down -> reset coil fires this many ms later (was 500 in v17)
// Backstop: if every target in a bank is physically down (switches closed) for this long and no reset was asked for recently, reset it
#define DROP_BANK_BACKSTOP_STABLE_MS  250
#define DROP_BANK_BACKSTOP_RETRY_MS   2500  // wait this long after any reset request before trying again
#define DROP_BANK_BACKSTOP_MAX_TRIES  3     // per bank per ball: stops a stuck switch from firing the coils forever
unsigned long BankAllDownSince[2];          // 0 = left bank, 1 = right bank
unsigned long BankLastResetRequest[2];
byte BankBackstopTries[2];
#define KNOCKER_SOLENOID_STRENGTH     5

// CUSTOMIZATION TODO:
// If your game has drop targets, set up objects here. Be sure to
// check out the types of target banks supported in DropTargtes.h
DropTargetBank LeftDropTargets(4, 2, DROP_TARGET_TYPE_BLY_1, DROP_TARGET_RESET_STRENGTH);
DropTargetBank RightDropTargets(4, 2, DROP_TARGET_TYPE_BLY_1, DROP_TARGET_RESET_STRENGTH);

byte PlayerUpLamps[4] = {LAMP_HEAD_PLAYER_1_UP, LAMP_HEAD_PLAYER_2_UP, LAMP_HEAD_PLAYER_3_UP, LAMP_HEAD_PLAYER_4_UP};


/******************************************************

   Adjustments Serialization

*/
void SetAllParameterDefaults() {

  // In the event that the EEPROM has not been initialized,
  // these are the values that will be used
  HighScore = 350000;                // factory high score to date
  Credits = 4;
  FreePlayMode = true;
  BallSaveNumSeconds = 0;            // Phoenix has no ball save
  MusicVolume = 10;
  SoundEffectsVolume = 10;
  CalloutsVolume = 10;
  AwardScores[0] = 330000;           // instruction card: 330,000 scores 1 credit
  AwardScores[1] = 500000;           // 500,000 scores 1 credit
  AwardScores[2] = 0;                // disabled
  TournamentScoring = false;
  MaxTiltWarnings = 2;
  ScoreAwardReplay = 0x03;           // both score awards are a credit, as on the instruction card
  BallsPerGame = 5;                  // 5-ball play (function 18 = 25)
  ScrollingScores = false;           // 6-digit display wraps at 999,999 instead
  MatchFeature = true;
  ExtraBallValue = 20000;
  SpecialValue = 40000;
  TimeRequiredToResetGame = 2;
  CPCSelection[0] = 4;
  CPCSelection[1] = 4;
  CPCSelection[2] = 4;

  // EASY / MEDIUM / HARD rules
  GameRulesSelection = GAME_RULES_MEDIUM;

  // CUSTOMIZATION TODO:
  // Set up default/reset values for any game
  // parameters that can be controlled through 
  // the operator menus
  BallServeSolenoidStrength = 10;
  SaucerSolenoidStrength = 5;
}


// CUSTOMIZATION TODO:
// Load in defaults for easy / medium / hard rules
boolean LoadRuleDefaults(byte ruleLevel) {
  // This function just puts the rules in RAM.
  // It's up to the caller to ensure that they're 
  // written to EEPROM when desired (with WriteParameters)
  // Phoenix has no ball save and no easy/medium/hard rule sets,
  // so every level is the same
  if (ruleLevel==GAME_RULES_EASY || ruleLevel==GAME_RULES_MEDIUM || ruleLevel==GAME_RULES_HARD) {
    BallSaveNumSeconds = 0;
  } else {
    return false;
  }

  return true;
}


void WriteParameters(boolean onlyWriteRulesParameters = true) {
  if (!onlyWriteRulesParameters) {
    RPU_WriteULToEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, HighScore);
    RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
    RPU_WriteByteToEEProm(RPU_CPC_CHUTE_1_SELECTION_BYTE, CPCSelection[0]);
    RPU_WriteByteToEEProm(RPU_CPC_CHUTE_2_SELECTION_BYTE, CPCSelection[1]);
    RPU_WriteByteToEEProm(RPU_CPC_CHUTE_3_SELECTION_BYTE, CPCSelection[2]);
        
    RPU_WriteByteToEEProm(EEPROM_FREE_PLAY_BYTE, FreePlayMode);
    RPU_WriteByteToEEProm(EEPROM_MUSIC_VOLUME_BYTE, MusicVolume);
    RPU_WriteByteToEEProm(EEPROM_SFX_VOLUME_BYTE, SoundEffectsVolume);
    RPU_WriteByteToEEProm(EEPROM_CALLOUTS_VOLUME_BYTE, CalloutsVolume);

    RPU_WriteULToEEProm(RPU_AWARD_SCORE_1_EEPROM_START_BYTE, AwardScores[0]);
    RPU_WriteULToEEProm(RPU_AWARD_SCORE_2_EEPROM_START_BYTE, AwardScores[1]);
    RPU_WriteULToEEProm(RPU_AWARD_SCORE_3_EEPROM_START_BYTE, AwardScores[2]);

    RPU_WriteByteToEEProm(EEPROM_TOURNAMENT_SCORING_BYTE, TournamentScoring);
    RPU_WriteByteToEEProm(EEPROM_AWARD_OVERRIDE_BYTE, ScoreAwardReplay);
    RPU_WriteByteToEEProm(EEPROM_BALLS_OVERRIDE_BYTE, BallsPerGame);
    RPU_WriteByteToEEProm(EEPROM_SCROLLING_SCORES_BYTE, ScrollingScores);
    RPU_WriteByteToEEProm(EEPROM_MATCH_FEATURE_BYTE, MatchFeature);
    
    RPU_WriteULToEEProm(EEPROM_EXTRA_BALL_SCORE_UL, ExtraBallValue);
    RPU_WriteULToEEProm(EEPROM_SPECIAL_SCORE_UL, SpecialValue);
    RPU_WriteByteToEEProm(EEPROM_CRB_HOLD_TIME, TimeRequiredToResetGame);

    // Set baseline for audits
    RPU_WriteByteToEEProm(RPU_CHUTE_1_COINS_START_BYTE, 0);
    RPU_WriteByteToEEProm(RPU_CHUTE_2_COINS_START_BYTE, 0);
    RPU_WriteByteToEEProm(RPU_CHUTE_3_COINS_START_BYTE, 0);
    RPU_WriteULToEEProm(RPU_TOTAL_PLAYS_EEPROM_START_BYTE, 0);
    RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, 0);
    RPU_WriteULToEEProm(RPU_TOTAL_HISCORE_BEATEN_START_BYTE, 0);
  }

  RPU_WriteByteToEEProm(EEPROM_BALL_SAVE_BYTE, BallSaveNumSeconds);
  RPU_WriteByteToEEProm(EEPROM_TILT_WARNING_BYTE, MaxTiltWarnings); 
  
  // CUSTOMIZATION TODO:
  // Write any custom parameters to EEPROM here
  RPU_WriteByteToEEProm(EEPROM_TROUGH_EJECT_STRENGTH, BallServeSolenoidStrength);
  RPU_WriteByteToEEProm(EEPROM_SAUCER_EJECT_STRENGTH, SaucerSolenoidStrength);
}

void ReadStoredParameters() {
  for (byte count = 0; count < 3; count++) {
    ChuteCoinsInProgress[count] = 0;
  }

  // The first time the EEPROM has been written with good values for this game,
  // the EEPROM_RPOS_INIT_PROOF_UL will be written to a known state (RPOS_INIT_PROOF)
  // if that value hasn't been written, then we load defaults and save them to EEPROM.
  // This should only happen the first time a device is run with this game code.
  unsigned long RPUProofValue = RPU_ReadULFromEEProm(EEPROM_RPOS_INIT_PROOF_UL, 0);
  if (RPUProofValue!=RPOS_INIT_PROOF) {
    // Doesn't look like this memory has been initialized
    RPU_WriteULToEEProm(EEPROM_RPOS_INIT_PROOF_UL, RPOS_INIT_PROOF);
    SetAllParameterDefaults();
    WriteParameters(false);
  } else {

    // Read machine settings
    HighScore = RPU_ReadULFromEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, 10000);
    Credits = RPU_ReadByteFromEEProm(RPU_CREDITS_EEPROM_BYTE);
    if (Credits > MaximumCredits) Credits = MaximumCredits;
  
    // This machine is always on free play: Start never needs a credit
    FreePlayMode = true;
    MusicVolume = ReadSetting(EEPROM_MUSIC_VOLUME_BYTE, 10, 10);
    SoundEffectsVolume = ReadSetting(EEPROM_SFX_VOLUME_BYTE, 10, 10);
    CalloutsVolume = ReadSetting(EEPROM_CALLOUTS_VOLUME_BYTE, 10, 10);
    Audio.SetMusicVolume(MusicVolume);
    Audio.SetSoundFXVolume(SoundEffectsVolume);
    Audio.SetNotificationsVolume(CalloutsVolume);

    AwardScores[0] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_1_EEPROM_START_BYTE);
    AwardScores[1] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_2_EEPROM_START_BYTE);
    AwardScores[2] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_3_EEPROM_START_BYTE);
  
    TournamentScoring = ReadSetting(EEPROM_TOURNAMENT_SCORING_BYTE, false, true);
    ScoreAwardReplay = ReadSetting(EEPROM_AWARD_OVERRIDE_BYTE, 0x03, 0x07);
    BallsPerGame = ReadSetting(EEPROM_BALLS_OVERRIDE_BYTE, 3, 10);
    ScrollingScores = ReadSetting(EEPROM_SCROLLING_SCORES_BYTE, true, true);
    ScrollingScores = false;   // Phoenix: scores wrap at 999,999; MillionsCount scores scroll their full total every 10 seconds
    MatchFeature = ReadSetting(EEPROM_MATCH_FEATURE_BYTE, true, true);

    CPCSelection[0] = ReadSetting(RPU_CPC_CHUTE_1_SELECTION_BYTE, 4, 8);
    CPCSelection[1] = ReadSetting(RPU_CPC_CHUTE_2_SELECTION_BYTE, 4, 8);
    CPCSelection[2] = ReadSetting(RPU_CPC_CHUTE_3_SELECTION_BYTE, 4, 8);
    CPCSelectionsHaveBeenRead = true;

    ExtraBallValue = RPU_ReadULFromEEProm(EEPROM_EXTRA_BALL_SCORE_UL);
    if (ExtraBallValue % 1000 || ExtraBallValue > 1000000) ExtraBallValue = 20000;
  
    SpecialValue = RPU_ReadULFromEEProm(EEPROM_SPECIAL_SCORE_UL);
    if (SpecialValue % 1000 || SpecialValue > 1000000) SpecialValue = 40000;
  
    TimeRequiredToResetGame = ReadSetting(EEPROM_CRB_HOLD_TIME, 1, 99);
    if (TimeRequiredToResetGame > 3 && TimeRequiredToResetGame != 99) TimeRequiredToResetGame = 1;
    
    // Read game rules
    GameRulesSelection = ReadSetting(EEPROM_GAME_RULES_SELECTION, GAME_RULES_MEDIUM, GAME_RULES_CUSTOM);

    BallSaveNumSeconds = ReadSetting(EEPROM_BALL_SAVE_BYTE, 15, 20);  
    MaxTiltWarnings = ReadSetting(EEPROM_TILT_WARNING_BYTE, 2, 3);
    
    BallServeSolenoidStrength = ReadSetting(EEPROM_TROUGH_EJECT_STRENGTH, 10, 60);
    SaucerSolenoidStrength = ReadSetting(EEPROM_SAUCER_EJECT_STRENGTH, 5, 15);
  }
}



byte GetCPCSelection(byte chuteNumber) {
  if (chuteNumber>2) return 0xFF;

  if (CPCSelectionsHaveBeenRead==false) {
    CPCSelection[0] = RPU_ReadByteFromEEProm(RPU_CPC_CHUTE_1_SELECTION_BYTE);
    if (CPCSelection[0]>=NUM_CPC_PAIRS) {
      CPCSelection[0] = 4;
      RPU_WriteByteToEEProm(RPU_CPC_CHUTE_1_SELECTION_BYTE, 4);
    }
    CPCSelection[1] = RPU_ReadByteFromEEProm(RPU_CPC_CHUTE_2_SELECTION_BYTE);  
    if (CPCSelection[1]>=NUM_CPC_PAIRS) {
      CPCSelection[1] = 4;
      RPU_WriteByteToEEProm(RPU_CPC_CHUTE_2_SELECTION_BYTE, 4);
    }
    CPCSelection[2] = RPU_ReadByteFromEEProm(RPU_CPC_CHUTE_3_SELECTION_BYTE);  
    if (CPCSelection[2]>=NUM_CPC_PAIRS) {
      CPCSelection[2] = 4;
      RPU_WriteByteToEEProm(RPU_CPC_CHUTE_3_SELECTION_BYTE, 4);
    }
    CPCSelectionsHaveBeenRead = true;
  }
  
  return CPCSelection[chuteNumber];
}


byte GetCPCCoins(byte cpcSelection) {
  if (cpcSelection>=NUM_CPC_PAIRS) return 1;
  return CPCPairs[cpcSelection][0];
}


byte GetCPCCredits(byte cpcSelection) {
  if (cpcSelection>=NUM_CPC_PAIRS) return 1;
  return CPCPairs[cpcSelection][1];
}


void QueueDIAGNotification(unsigned short notificationNum) {
  // This is optional, but the machine can play an audio message at boot
  // time to indicate any errors and whether it's going to boot to original
  // or new code.
  //Audio.QueuePrioritizedNotification(notificationNum, 0, 10, CurrentTime);
  (void)notificationNum;
}

// CUSTOMIZATION TODO:
// This function only needs to be updated if the display
// numbers (in test mode) don't match the indices.
// In most games, the operator friendly numbers are just
// the display numbers + 1
// I'm doing this as a function instead of an array because 
// memory is short and spending 44 bytes on a converstion array
// seems wasteful when the board has tons and tons of code space.
// There's a way to store this data in code space and then convert
// it when needed, but that's slow compared to this (ugly) method.
byte LampConvertDisplayNumberToIndex(byte displayNumber) {

  if (displayNumber<60) return displayNumber + 1;
  return OPERATOR_MENU_VALUE_OUT_OF_RANGE;
}


// CUSTOMIZATION TODO:
// Fill in the "operator friendly" solenoid number in the "case" 
// (this is the number reported in the game manual)
// and then the hardware index in the return (from your defines)
unsigned short SolenoidConvertDisplayNumberToIndex(byte displayNumber) {
  // Phoenix solenoid numbers (handbook Figure 10 / test 02): 1-22, with 8, 15 and 22 unused.
  // The hardware index is the number minus one.
  if (displayNumber==0 || displayNumber==8 || displayNumber==15 || displayNumber==22) return OPERATOR_MENU_VALUE_UNUSED;
  if (displayNumber<=22) return displayNumber - 1;
  return OPERATOR_MENU_VALUE_OUT_OF_RANGE;
}

// CUSTOMIZATION TODO:
// In the operator menu's test mode, these are the strengths
// that will be used 
byte SolenoidConvertDisplayNumberToTestStrength(byte displayNumber) {
  if (displayNumber==0 || displayNumber==8 || displayNumber==15 || displayNumber==22) return OPERATOR_MENU_VALUE_UNUSED;
  if (displayNumber==1) return BallServeSolenoidStrength;
  if (displayNumber>=2 && displayNumber<=7) return DROP_TARGET_RESET_STRENGTH;
  if (displayNumber<=22) return 4;
  return OPERATOR_MENU_VALUE_OUT_OF_RANGE;
}




////////////////////////////////////////////////////////////////////////////
//
//  Setup
//    Arduino calls this function at power up and reset.
//    It's used to initialize the hardware and 
//    certain variables and structures used by 
//    the code.
//
////////////////////////////////////////////////////////////////////////////

void setup() {

  if (DEBUG_MESSAGES) {
    // If debug is on, set up the Serial port for communication
    Serial.begin(115200);
    Serial.write("Starting\n");
  }

  // Set up the Audio handler in order to play boot messages
  CurrentTime = millis();
  Audio.InitDevices(AUDIO_PLAY_TYPE_WAV_TRIGGER | AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
  Audio.StopAllAudio();
  Audio.SetMusicDuckingGain(25);
  Audio.SetSoundFXDuckingGain(20);

  // Tell the OS about game-specific switches
  // (this is for software-controlled pop bumpers and slings)
#if (RPU_MPU_ARCHITECTURE<10)
  // Machines with a -17, -35, 100, and 200 architecture
  // almost always have software based switch-triggered solenoids.
  // For those, you can define an array of solenoids and the switches
  // that will trigger them:
  RPU_SetupGameSwitches(NUM_SWITCHES_WITH_TRIGGERS, NUM_PRIORITY_SWITCHES_WITH_TRIGGERS, SolenoidAssociatedSwitches);

#endif

  // Set up the chips and interrupts
  unsigned long initResult = 0;
  if (DEBUG_MESSAGES) Serial.write("Initializing MPU\n");
  Serial.write("PhoenixRPU build 21\n");

  // If the hardware has the ability to switch on the Credit/Reset button (requires Rev 4 or greater)
  // then that can be used to choose Original or New code. Otherwise, the hardware switch
  // will choose Original if open, and New if closed
  initResult = RPU_InitializeMPU(   RPU_CMD_BOOT_ORIGINAL_IF_CREDIT_RESET | RPU_CMD_BOOT_ORIGINAL_IF_NOT_SWITCH_CLOSED |
                                    RPU_CMD_INIT_AND_RETURN_EVEN_IF_ORIGINAL_CHOSEN | RPU_CMD_PERFORM_MPU_TEST, SW_CREDIT_RESET);

  // Tell the System Alpha how long to pulse the drop-target reset coils (it ignores the strength numbers)
  RPU_SetSolenoidDefaultPulse(SOL_DROP_L_TOP_RESET, DROP_TARGET_RESET_PULSE_MS);
  RPU_SetSolenoidDefaultPulse(SOL_DROP_L_BOTTOM_RESET, DROP_TARGET_RESET_PULSE_MS);
  RPU_SetSolenoidDefaultPulse(SOL_DROP_R_TOP_RESET, DROP_TARGET_RESET_PULSE_MS);
  RPU_SetSolenoidDefaultPulse(SOL_DROP_R_BOTTOM_RESET, DROP_TARGET_RESET_PULSE_MS);
  RPU_SetSolenoidDefaultPulse(SOL_DROP_CENTER_LEFT_RESET, DROP_TARGET_RESET_PULSE_MS);
  RPU_SetSolenoidDefaultPulse(SOL_DROP_CENTER_RIGHT_RESET, DROP_TARGET_RESET_PULSE_MS);

  if (DEBUG_MESSAGES) {
    char buf[128];
    sprintf(buf, "Return from init = 0x%04lX\n", initResult);
    Serial.write(buf);
    if (initResult & RPU_RET_6800_DETECTED) Serial.write("Detected 6800 clock\n");
    else if (initResult & RPU_RET_6802_OR_8_DETECTED) Serial.write("Detected 6802/8 clock\n");
    Serial.write("Back from init\n");
  }

  if (initResult & RPU_RET_SELECTOR_SWITCH_ON) QueueDIAGNotification(SOUND_EFFECT_DIAG_SELECTOR_SWITCH_ON);
  else QueueDIAGNotification(SOUND_EFFECT_DIAG_SELECTOR_SWITCH_OFF);

  if (initResult & RPU_RET_CREDIT_RESET_BUTTON_HIT) QueueDIAGNotification(SOUND_EFFECT_DIAG_CREDIT_RESET_BUTTON);

  if (initResult & RPU_RET_DIAGNOSTIC_REQUESTED) {
    QueueDIAGNotification(SOUND_EFFECT_DIAG_STARTING_DIAGNOSTICS);
    // Run diagnostics here:
  }

  if (initResult & RPU_RET_ORIGINAL_CODE_REQUESTED) {
    if (DEBUG_MESSAGES) Serial.write("Asked to run original code\n");
    delay(100);
    QueueDIAGNotification(SOUND_EFFECT_DIAG_STARTING_ORIGINAL_CODE);
    delay(100);

#ifdef RPU_OS_USE_ACCESSORY_LAMP_BOARD
    TopperALB.InitOutogingCommunication();
    TopperALB.SetTargetDeviceAddress(8);
    TopperALB.EnableLamps();
    TopperALB.WatchdogTimerReset();
    TopperALB.AdjustSetting(SET_COLOR_0, 0, 0, 200);
    TopperALB.AdjustSetting(SET_COLOR_1, 200, 200, 0);
    ALBWatchdogResetLastSent = millis();
#endif

    while (Audio.Update(millis()));
    // Arduino should hang if original code is running
    while (1) {
#ifdef RPU_OS_USE_ACCESSORY_LAMP_BOARD
      // Send a periodic message to keep GI on
      if (GIInOriginal) {
        if (millis() >  (ALBWatchdogResetLastSent + ALB_WATCHDOG_TIMER_DURATION/2)) {
          TopperALB.WatchdogTimerReset();
          ALBWatchdogResetLastSent = millis();
        }
      }
#endif
    }
  }
  QueueDIAGNotification(SOUND_EFFECT_DIAG_STARTING_NEW_CODE);

  RPU_DisableSolenoidStack();
  RPU_SetDisableFlippers(true);

  // Read parameters from EEProm
  ReadStoredParameters();

  CurrentScores[0] = GAME_MAJOR_VERSION;
  CurrentScores[1] = GAME_MINOR_VERSION;
  CurrentScores[2] = RPU_OS_MAJOR_VERSION;
  CurrentScores[3] = RPU_OS_MINOR_VERSION;

  CurrentAchievements[0] = 0;
  CurrentAchievements[1] = 0;
  CurrentAchievements[2] = 0;
  CurrentAchievements[3] = 0;

  // CUSTOMIZATION TODO:
  // If your game has drop targets, this is a good place
  // to configure the drop target switches and solenoids
  // Initialize any drop target variables here
  LeftDropTargets.DefineSwitch(0, SW_DROP_L_1);
  LeftDropTargets.DefineSwitch(1, SW_DROP_L_2);
  LeftDropTargets.DefineSwitch(2, SW_DROP_L_3);
  LeftDropTargets.DefineSwitch(3, SW_DROP_L_4);
  LeftDropTargets.DefineResetSolenoid(0, SOL_DROP_L_BOTTOM_RESET);
  LeftDropTargets.DefineResetSolenoid(1, SOL_DROP_L_TOP_RESET);
  RightDropTargets.DefineSwitch(0, SW_DROP_R_1);
  RightDropTargets.DefineSwitch(1, SW_DROP_R_2);
  RightDropTargets.DefineSwitch(2, SW_DROP_R_3);
  RightDropTargets.DefineSwitch(3, SW_DROP_R_4);
  RightDropTargets.DefineResetSolenoid(0, SOL_DROP_R_BOTTOM_RESET);
  RightDropTargets.DefineResetSolenoid(1, SOL_DROP_R_TOP_RESET);

  CurrentTime = millis();

  // CUSTOMIZATION TODO:
  // You can play a machine startup sound here
  Audio.QueueSound(SOUND_EFFECT_STARTUP_1 + (micros()) % 2, AUDIO_PLAY_TYPE_WAV_TRIGGER, CurrentTime + 1200);
  OperatorSwitchPressStarted = 0;
  InOperatorMenu = false;

  // CUSTOMIZATION TODO:
  // The operator menus can use different configurations for navigation buttons.
  // I've left a commented out examples from Black Knight and Firepower to show how forward/back buttons
  // can be configured if you have them
  Menus.SetNavigationButtons(SWITCH_STACK_EMPTY, SWITCH_STACK_EMPTY, SW_CREDIT_RESET, SW_SELF_TEST_SWITCH);
  // Menus.SetNavigationButtons(SW_RIGHT_MAGNET_BUTTON, SW_LEFT_MAGNET_BUTTON, SW_CREDIT_RESET, SW_SELF_TEST_SWITCH);
  // Menus.SetNavigationButtons(SW_RIGHT_FLIPPER, SW_HIGH_SCORE_RESET, SW_CREDIT_RESET, SW_SELF_TEST_SWITCH);

  Menus.SetLampsLookupCallback(LampConvertDisplayNumberToIndex);
  Menus.SetSolenoidIDLookupCallback(SolenoidConvertDisplayNumberToIndex);
  Menus.SetSolenoidStrengthLookupCallback(SolenoidConvertDisplayNumberToTestStrength);
  Menus.SetMenuButtonDebounce(250);

#ifdef RPU_OS_USE_ACCESSORY_LAMP_BOARD
  TopperALB.InitOutogingCommunication();
  TopperALB.SetTargetDeviceAddress(8);
  TopperALB.EnableLamps();
  TopperALB.WatchdogTimerReset();
  TopperALB.AdjustSetting(SET_COLOR_0, 0, 0, 200);
  TopperALB.AdjustSetting(SET_COLOR_1, 200, 200, 0);
  ALBWatchdogResetLastSent = millis();
#endif

  LastTiltWarningTime = 0;
}

byte ReadSetting(byte setting, byte defaultValue, byte maxValue) {
  byte value = EEPROM.read(setting);
  if (value == 0xFF || value>maxValue) {
    EEPROM.write(setting, defaultValue);
    return defaultValue;
  }
  return value;
}

// This function is useful for checking the status of drop target switches
byte CheckSequentialSwitches(byte startingSwitch, byte numSwitches) {
  byte returnSwitches = 0;
  for (byte count = 0; count < numSwitches; count++) {
    returnSwitches |= (RPU_ReadSingleSwitchState(startingSwitch + count) << count);
  }
  return returnSwitches;
}

// CUSTOMIZATION TODO:
// Set up lamp functions so the lamps will reflect
// the current game state. For organization, create a different
// function for groups of similar lamps, like ShowTopLanes()
////////////////////////////////////////////////////////////////////////////
//
//  Lamp Management functions
//    These functions are called each time through the gameplay loop.
//    They use the current status variables to set each lamp to 
//    on, off, dim (depending on hardware), or flashing. The lamps are
//    actually set in hardware by the Interrupt Service Routine, so
//    these functions simply update the state arrays used by the ISR.
//
//    If a special animation is required (sweeps and such), then these
//    functions are turned off and one of the special functions in
//    "LampAnimations.h" is used to set those lamps.
//
////////////////////////////////////////////////////////////////////////////
void SetGeneralIlluminationOn(boolean setGIOn = true) {
  // Since this machine doesn't have GI control,
  // this line prevents compiler warnings.
  (void)setGIOn;
}

void ShowPlayerLamps() {  
  for (byte count = 0; count < 4; count++) {
    if (count==CurrentPlayer) RPU_SetLampState(PlayerUpLamps[count], 1, 0, 250);
    else if (count<CurrentNumPlayers) RPU_SetLampState(PlayerUpLamps[count], 1);
    else RPU_SetLampState(PlayerUpLamps[count], 0);
  }  
}

// Backbox status lamps: GAME OVER, 1-4 CAN PLAY, BALL IN PLAY.
// inGame = true while a game is running (including a tilt); false in game over / attract.
//  - Game over: GAME OVER lit, BALL IN PLAY off, "1 CAN PLAY" lit if a credit (or free play) is available.
//  - In a game: GAME OVER off, BALL IN PLAY lit, only the lamp for the number of players in this
//    game is lit (handbook p.5: more players "change the number of players light").
// These lamps (48-63) sit outside the attract light show (lamps 0-47), so nothing else touches them.
void SetBackboxStatusLamps(boolean inGame) {
  byte canPlayLamps[4] = {LAMP_HEAD_1_PLAYER, LAMP_HEAD_2_PLAYERS, LAMP_HEAD_3_PLAYERS, LAMP_HEAD_4_PLAYERS};

  RPU_SetLampState(LAMP_HEAD_GAME_OVER, inGame ? 0 : 1);
  RPU_SetLampState(LAMP_BALL_IN_PLAY, inGame ? 1 : 0);

  for (byte count = 0; count < 4; count++) {
    boolean lit;
    if (inGame) lit = (CurrentNumPlayers > 0) && (count == (CurrentNumPlayers - 1));
    else lit = (count == 0) && (FreePlayMode || Credits > 0);
    RPU_SetLampState(canPlayLamps[count], lit ? 1 : 0);
  }
}

byte BonusLampAssignments[11] = { LAMP_BONUS_1K, LAMP_BONUS_2K, LAMP_BONUS_3K, LAMP_BONUS_4K, LAMP_BONUS_5K, LAMP_BONUS_6K, 
                                  LAMP_BONUS_7K, LAMP_BONUS_8K, LAMP_BONUS_9K, LAMP_BONUS_10K, LAMP_BONUS_20K};

void ShowBonusLamps() {
  RPU_SetLampState(LAMP_BONUS_20K, Bonus[CurrentPlayer]>=20);
  RPU_SetLampState(LAMP_BONUS_10K, Bonus[CurrentPlayer]>=10 && Bonus[CurrentPlayer]<20);
  for (byte count=1; count<10; count++) {
    RPU_SetLampState(BonusLampAssignments[count-1], (Bonus[CurrentPlayer]%10)==count);
  }
  byte bonusX = BonusX[CurrentPlayer];
  RPU_SetLampState(LAMP_X2, bonusX==2);
  RPU_SetLampState(LAMP_X3, bonusX==3);
  RPU_SetLampState(LAMP_X4, bonusX==4);
  RPU_SetLampState(LAMP_X5, bonusX==5);
}



void ShowShootAgainLamp() {

  if ( (BallFirstSwitchHitTime == 0 && BallSaveNumSeconds) || (BallSaveEndTime && CurrentTime < BallSaveEndTime) ) {
    // Ball save is active: fast blink on both lamps (unchanged behaviour)
    unsigned long msRemaining = 5000;
    if (BallSaveEndTime != 0) msRemaining = BallSaveEndTime - CurrentTime;
    unsigned int flashPeriod = (msRemaining < 5000) ? 100 : 500;
    RPU_SetLampState(LAMP_SHOOT_AGAIN, 1, 0, flashPeriod);
    RPU_SetLampState(LAMP_HEAD_SHOOT_AGAIN, 1, 0, flashPeriod);
    return;
  }

  byte numBanked = ExtraBallsBanked[CurrentPlayer];
  if (numBanked > MAX_EXTRA_BALLS) numBanked = MAX_EXTRA_BALLS;
  boolean lampOn = false;

  if (numBanked) {
    // N blinks, then a pause. The last blink has no dark half of its own, so the
    // dark gap between sequences is exactly SHOOT_AGAIN_PAUSE_MS.
    unsigned long sequenceMs = ((unsigned long)numBanked * SHOOT_AGAIN_BLINK_SLOT_MS) - (SHOOT_AGAIN_BLINK_SLOT_MS - SHOOT_AGAIN_BLINK_ON_MS);
    unsigned long cycleMs = sequenceMs + SHOOT_AGAIN_PAUSE_MS;
    unsigned long phase = (CurrentTime - ShootAgainCycleStart) % cycleMs;
    lampOn = (phase < sequenceMs) && ((phase % SHOOT_AGAIN_BLINK_SLOT_MS) < SHOOT_AGAIN_BLINK_ON_MS);
  }

  RPU_SetLampState(LAMP_SHOOT_AGAIN, lampOn);
  RPU_SetLampState(LAMP_HEAD_SHOOT_AGAIN, lampOn);
}





////////////////////////////////////////////////////////////////////////////
//
//  Machine State Helper functions
//
////////////////////////////////////////////////////////////////////////////
boolean AddPlayer(boolean resetNumPlayers = false) {

  if (Credits < 1 && !FreePlayMode) return false;
  if (resetNumPlayers) CurrentNumPlayers = 0;
  if (CurrentNumPlayers >= RPU_NUMBER_OF_PLAYERS_ALLOWED) return false;

  CurrentNumPlayers += 1;
  RPU_SetDisplay(CurrentNumPlayers - 1, 0, true, 2);
  
  if (CurrentNumPlayers > 1) {
    // If this is second, third, or fourth player, then playe the announcment
    QueueNotification(SOUND_EFFECT_VP_ADD_PLAYER_1 + (CurrentNumPlayers - 1), 10);
  }

  for (byte count = 0; count < 4; count++) {
    if (count==CurrentPlayer) RPU_SetLampState(PlayerUpLamps[count], 1, 0, 250);
    else if (count<CurrentNumPlayers) RPU_SetLampState(PlayerUpLamps[count], 1);
    else RPU_SetLampState(PlayerUpLamps[count], 0);
  }

  // Move the CAN PLAY lamp to the new player count
  SetBackboxStatusLamps(true);

  if (!FreePlayMode) {
    Credits -= 1;
    RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    RPU_SetCoinLockout(false);
  }

  RPU_WriteULToEEProm(RPU_TOTAL_PLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_PLAYS_EEPROM_START_BYTE) + 1);

  return true;
}


unsigned short ChuteAuditByte[] = {RPU_CHUTE_1_COINS_START_BYTE, RPU_CHUTE_2_COINS_START_BYTE, RPU_CHUTE_3_COINS_START_BYTE};
void AddCoinToAudit(byte chuteNum) {
  if (chuteNum > 2) return;
  unsigned short coinAuditStartByte = ChuteAuditByte[chuteNum];
  RPU_WriteULToEEProm(coinAuditStartByte, RPU_ReadULFromEEProm(coinAuditStartByte) + 1);
}


void AddCredit(boolean playSound = false, byte numToAdd = 1) {
  if (Credits < MaximumCredits) {
    Credits += numToAdd;
    if (Credits > MaximumCredits) Credits = MaximumCredits;
    RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
    if (playSound) {
      //PlaySoundEffect(SOUND_EFFECT_ADD_CREDIT);
      RPU_PushToSolenoidStack(SOL_KNOCKER, KNOCKER_SOLENOID_STRENGTH, true);
    }
    RPU_SetDisplayCredits(Credits, true);
    RPU_SetCoinLockout(false);
  } else {
    RPU_SetDisplayCredits(Credits, true);
    RPU_SetCoinLockout(true);
  }

}

byte SwitchToChuteNum(byte switchHit) {
  (void)switchHit;
  byte chuteNum = 0;
  return chuteNum;
}

boolean AddCoin(byte chuteNum) {
  boolean creditAdded = false;
  if (chuteNum > 2) return false;
  byte cpcSelection = GetCPCSelection(chuteNum);

  // Find the lowest chute num with the same ratio selection
  // and use that ChuteCoinsInProgress counter
  byte chuteNumToUse;
  for (chuteNumToUse = 0; chuteNumToUse <= chuteNum; chuteNumToUse++) {
    if (GetCPCSelection(chuteNumToUse) == cpcSelection) break;
  }

  PlaySoundEffect(SOUND_EFFECT_COIN_DROP_1 + (CurrentTime % 3));

  byte cpcCoins = GetCPCCoins(cpcSelection);
  byte cpcCredits = GetCPCCredits(cpcSelection);
  byte coinProgressBefore = ChuteCoinsInProgress[chuteNumToUse];
  ChuteCoinsInProgress[chuteNumToUse] += 1;

  if (ChuteCoinsInProgress[chuteNumToUse] == cpcCoins) {
    if (cpcCredits > cpcCoins) AddCredit(cpcCredits - (coinProgressBefore));
    else AddCredit(cpcCredits);
    ChuteCoinsInProgress[chuteNumToUse] = 0;
    creditAdded = true;
  } else {
    if (cpcCredits > cpcCoins) {
      AddCredit(1);
      creditAdded = true;
    } else {
    }
  }

  return creditAdded;
}


void AddSpecialCredit() {
  AddCredit(false, 1);
  RPU_PushToTimedSolenoidStack(SOL_KNOCKER, KNOCKER_SOLENOID_STRENGTH, CurrentTime, true);
  RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE) + 1);
}

void AwardSpecial() {
  if (SpecialCollected) return;
  SpecialCollected = true;
  if (TournamentScoring) {
    CurrentScores[CurrentPlayer] += SpecialValue * PlayfieldMultiplier;
  } else {
    AddSpecialCredit();
  }
}

// The "Extra Ball When Lit" lamp is lit while an extra ball is waiting at the bullseye
// and there is room in the bank to take it.
boolean ExtraBallIsLit() {
  return ExtraBallAvailable[CurrentPlayer]==1 && ExtraBallsAwarded[CurrentPlayer] < MAX_EXTRA_BALLS;
}

// Lit side of the alternating outside rollovers: left on even periods, right on odd ones.
boolean LeftOutlaneSpecialIsLit() {
  return ((CurrentTime / OUTLANE_SPECIAL_ALTERNATE_MS) % 2) == 0;
}

// The spinner is lit (1000 points) while the bonus is 10,000 through 19,000.
boolean SpinnerIsLit() {
  return Bonus[CurrentPlayer] >= 10 && Bonus[CurrentPlayer] < 20;
}

void ShowPhoenixLamps() {
  byte player = CurrentPlayer;
  for (byte count = 0; count < 6; count++) RPU_SetLampState(PhoenixLetterLamps[count], count < PhoenixLetters[player]);
  RPU_SetLampState(LAMP_X, PhoenixXLit[player]);
  // Completed PHOENIX at least once: blink all seven lamps once per completion every 30 s, then show progress again
  if (PhoenixCompletions[player]) {
    unsigned long phase = (CurrentTime - PhoenixCompleteTime[player]) % PHOENIX_BLINK_INTERVAL_MS;
    if (phase < (unsigned long)PhoenixCompletions[player] * PHOENIX_BLINK_PERIOD_MS) {
      boolean blinkOn = (phase % PHOENIX_BLINK_PERIOD_MS) < (PHOENIX_BLINK_PERIOD_MS / 2);
      for (byte count = 0; count < 6; count++) RPU_SetLampState(PhoenixLetterLamps[count], blinkOn);
      RPU_SetLampState(LAMP_X, blinkOn);
    }
  }
  for (byte count = 0; count < 5; count++) RPU_SetLampState(RolloverLamps[count], (RolloversLit[player] >> count) & 0x01);

  RPU_SetLampState(LAMP_EXTRA_BALL_WHEN_LIT, ExtraBallIsLit());
  RPU_SetLampState(LAMP_SPINNER_1000_WL, SpinnerIsLit());

  if (OutlaneSpecialReady[player] && ExtraBallsAwarded[player] < MAX_EXTRA_BALLS) {
    boolean leftLit = LeftOutlaneSpecialIsLit();
    RPU_SetLampState(LAMP_LEFT_SPECIAL, leftLit);
    RPU_SetLampState(LAMP_RIGHT_SPECIAL, !leftLit);
  } else {
    RPU_SetLampState(LAMP_LEFT_SPECIAL, 0);
    RPU_SetLampState(LAMP_RIGHT_SPECIAL, 0);
  }
}

// AwardExtraBall: awards that come from the lit Extra Ball When Lit lamp
// (ExtraBallAvailable==1, call with basedOnScore=false) and awards from specials or
// score thresholds (basedOnScore=true, leaves the lit lamp alone) both go into the bank. Returns true if a ball was added (or its value scored in
// tournament mode), false if there was nothing to award or the bank is full.
boolean AwardExtraBall(boolean basedOnScore = false) {
  if (!(ExtraBallAvailable[CurrentPlayer]==1 || basedOnScore)) return false;

  if (TournamentScoring) {
    if (!basedOnScore) ExtraBallAvailable[CurrentPlayer] = 0;
    CurrentScores[CurrentPlayer] += ExtraBallValue * PlayfieldMultiplier;
    return true;
  }

  if (ExtraBallsAwarded[CurrentPlayer] >= MAX_EXTRA_BALLS) return false;   // 5 extra balls per player per game, in total

  if (!basedOnScore) ExtraBallAvailable[CurrentPlayer] = 0;    // collecting the lit ball turns the lamp off
  ExtraBallsBanked[CurrentPlayer] += 1;
  ExtraBallsAwarded[CurrentPlayer] += 1;
  ShootAgainCycleStart = CurrentTime;       // restart the blink pattern so the new count shows right away
  QueueNotification(SOUND_EFFECT_VP_EXTRA_BALL, 8);
  return true;
}

// Scrolls the whole score (millions + 6 digits) across the display of every player over a million, then rests on the
// normal score for MILLIONS_PAUSE_MS and repeats. Call after Display_UpdateDisplays each pass; only the display is
// changed, never the score. numPlayers = how many displays to look at.
void ShowMillionsScroll(byte numPlayers) {
  for (byte player = 0; player < RPU_NUMBER_OF_PLAYERS_ALLOWED; player++) {
    boolean scrolling = false;
    if (player < numPlayers && MillionsCount[player] > 0) {
      // digits of the total score: the millions, then the six digits of the wrapped score
      char digits[12];
      byte numDigits = 0;
      unsigned long millions = MillionsCount[player];
      char millionsText[4];
      byte millionsLen = 0;
      while (millions) { millionsText[millionsLen++] = '0' + (millions % 10); millions /= 10; }
      while (millionsLen) digits[numDigits++] = millionsText[--millionsLen];
      unsigned long score = CurrentScores[player];
      for (byte place = 0; place < 6; place++) digits[numDigits + 5 - place] = '0' + ((score / (place == 0 ? 1UL : (place == 1 ? 10UL : (place == 2 ? 100UL : (place == 3 ? 1000UL : (place == 4 ? 10000UL : 100000UL)))))) % 10);
      numDigits += 6;

      // Frames 0 .. numDigits-2 scroll the digits in from the right; the last frame is the normal score
      unsigned long scrollMs = (unsigned long)(numDigits - 1) * MILLIONS_SCROLL_STEP_MS;
      unsigned long phase = (CurrentTime - MillionsScrollStart[player]) % (scrollMs + MILLIONS_PAUSE_MS);
      if (phase < scrollMs) {
        byte frame = phase / MILLIONS_SCROLL_STEP_MS;               // 0 .. numDigits-2
        byte shown = (frame + 1 > 6) ? 6 : (frame + 1);             // digits visible in this frame
        unsigned long value = 0;
        for (byte count = 0; count < shown; count++) value = value * 10 + (digits[frame - (shown - 1) + count] - '0');
        RPU_SetDisplay(player, value, false, 1);                    // write the digits...
        RPU_SetDisplayBlank(player, (byte)(((1 << shown) - 1) << (6 - shown)));   // ...and light only the right-hand ones
        scrolling = true;
      }
    }
    if (scrolling) {
      MillionsBlinkDark[player] = true;
    } else if (MillionsBlinkDark[player]) {
      RPU_SetDisplay(player, CurrentScores[player], true, 2, true);   // back to the normal score
      MillionsBlinkDark[player] = false;
    }
  }
}

// Called when all five "1"-"5" rollover lamps are lit. Spots the next letter(s).
void SpotLetters() {
  byte player = CurrentPlayer;
  // Every letter and X already lit and 1-5 completed again: PHOENIX is complete. Turn every letter off and start over.
  if (PhoenixLetters[player] >= 6 && PhoenixXLit[player]) {
    if (PhoenixCompletions[player] < 250) PhoenixCompletions[player] += 1;
    PhoenixCompleteTime[player] = CurrentTime;       // starts the first blink right away
    PhoenixLetters[player] = 0;
    PhoenixXLit[player] = false;
    OutlaneSpecialReady[player] = false;
    if (DEBUG_MESSAGES) Serial.write("PHOENIX complete: all letters off, start over\n");
    return;
  }

  if (PhoenixSets[player] < 250) PhoenixSets[player] += 1;

  if (PhoenixLetters[player] < 6) {
    byte toSpot = 1;   // every completion of 1-5 spots exactly one letter: P, H, O, E, N, I, then X
    for (byte count = 0; count < toSpot && PhoenixLetters[player] < 6; count++) {
      PhoenixLetters[player] += 1;
      if (PhoenixLetters[player] == PHOENIX_LETTER_E) ExtraBallAvailable[player] = 1;     // E lights Extra Ball When Lit
      if (PhoenixLetters[player] == PHOENIX_LETTER_I) OutlaneSpecialReady[player] = true; // I arms the outside rollovers
    }
  } else {
    // Every letter through I is lit: this set spots X, or turns X (and the outside specials) back on
    PhoenixXLit[player] = true;
    OutlaneSpecialReady[player] = true;
  }

  // Extra Ball When Lit is lit only when E is spotted (above), not on every later completion of 1-5
  if (DEBUG_MESSAGES) {
    char buf[40];
    sprintf(buf, "Letters lit: %d, X %d\n", PhoenixLetters[player], PhoenixXLit[player] ? 1 : 0);
    Serial.write(buf);
  }
}

void ResetCenterDropTarget(boolean left) {
  RPU_PushToTimedSolenoidStack(left ? SOL_DROP_CENTER_LEFT_RESET : SOL_DROP_CENTER_RIGHT_RESET, DROP_TARGET_RESET_STRENGTH, CurrentTime + 100, true);
}

// Lights one of the "1"-"5" lamps and spots a letter when all five are lit.
// "1" and "5" are both spotted by making either one.
void LightNumber(byte index) {
  byte player = CurrentPlayer;
  RolloversLit[player] |= (1 << index);
  if (index == 0 || index == 4) RolloversLit[player] |= 0x11;
  if (DEBUG_MESSAGES) {
    char buf[40];
    sprintf(buf, "Number %d lit, lamps now %02X\n", index + 1, RolloversLit[player] & 0x1F);
    Serial.write(buf);
  }
  if ((RolloversLit[player] & 0x1F) == 0x1F) {
    RolloversLit[player] = 0;
    if (DEBUG_MESSAGES) Serial.write("1-5 complete: number lamps cleared\n");
    SpotLetters();
  }
}

// "1" through "5" rollovers (index 0-4): 1000 and advance bonus.
// "2" resets the left centre drop target (A) and "4" the right one (B).
void HandleNumberRollover(byte index) {
  if (index == 1) ResetCenterDropTarget(true);
  if (index == 3) ResetCenterDropTarget(false);
  CurrentScores[CurrentPlayer] += SCORE_NUMBER_ROLLOVER * PlayfieldMultiplier;
  AddToBonus(1);
  LightNumber(index);
}

// Bullseye (switch 21): 5000 and one bonus advance, plus any extra ball or special that is waiting.
void HandleBullseyeHit() {
  byte player = CurrentPlayer;
  unsigned long bullseyeScore = SCORE_BULLSEYE;
  for (byte count = 0; count < PhoenixLetters[player]; count++) {
    if (PhoenixLetterValues[count] > bullseyeScore) bullseyeScore = PhoenixLetterValues[count];
  }
  if (PhoenixXLit[player] && PhoenixLetterValues[6] > bullseyeScore) bullseyeScore = PhoenixLetterValues[6];
  CurrentScores[player] += bullseyeScore * PlayfieldMultiplier;
  AddToBonus(1);
  if (ExtraBallIsLit()) {
    if (AwardExtraBall(false)) CurrentScores[player] += SCORE_BULLSEYE_EXTRA_BALL;   // collects the lit extra ball
  } else if (PhoenixXLit[player] && ExtraBallsAwarded[player] < MAX_EXTRA_BALLS) {
    if (AwardExtraBall(true)) PhoenixXLit[player] = false;  // special (an extra ball); X goes off
  }
}

// Outside rollover special, configured to award an extra ball. Only the lit side collects.
void HandleOutlaneSpecial(boolean leftOutlane) {
  if (!OutlaneSpecialReady[CurrentPlayer]) return;
  if (leftOutlane != LeftOutlaneSpecialIsLit()) return;
  // The special turns off once collected; the next completion of 1-5 lights it again
  if (AwardExtraBall(true)) OutlaneSpecialReady[CurrentPlayer] = false;
}


void IncreasePlayfieldMultiplier(unsigned long duration) {
  PlayfieldMultiplierTimeLeft += duration;

  PlayfieldMultiplier += 1;
  if (PlayfieldMultiplier > 5) {
    PlayfieldMultiplier = 5;
  }
}


void SetBallSave(unsigned long duration, byte numberOfSaves = 0xFF, boolean addToBallSave = false) {

  if (duration == 0) {
    BallSaveEndTime = 0;
    NumberOfBallSavesRemaining = 0;
  } else if (addToBallSave) {
    if (BallSaveEndTime) BallSaveEndTime += duration;
  } else {
    BallSaveEndTime = CurrentTime + duration;
    NumberOfBallSavesRemaining = numberOfSaves;
  }
}




#define SOUND_EFFECT_OM_CPC_VALUES                  180
#define SOUND_EFFECT_OM_CRB_VALUES                  210
#define SOUND_EFFECT_OM_DIFFICULTY_VALUES           220

#define SOUND_EFFECT_AP_TOP_LEVEL_MENU_ENTRY    1700
#define SOUND_EFFECT_AP_TEST_MENU               1701
#define SOUND_EFFECT_AP_AUDITS_MENU             1702
#define SOUND_EFFECT_AP_BASIC_ADJUSTMENTS_MENU  1703
#define SOUND_EFFECT_AP_GAME_RULES_LEVEL        1704
#define SOUND_EFFECT_AP_GAME_SPECIFIC_ADJ_MENU  1705

#define SOUND_EFFECT_AP_TEST_LAMPS              1710
#define SOUND_EFFECT_AP_TEST_DISPLAYS           1711
#define SOUND_EFFECT_AP_TEST_SOLENOIDS          1712
#define SOUND_EFFECT_AP_TEST_SWITCHES           1713
#define SOUND_EFFECT_AP_TEST_SOUNDS             1714
#define SOUND_EFFECT_AP_TEST_EJECT_BALLS        1715

#define SOUND_EFFECT_AP_AUDIT_TOTAL_PLAYS       1720
#define SOUND_EFFECT_AP_AUDIT_CHUTE_1_COINS     1721
#define SOUND_EFFECT_AP_AUDIT_CHUTE_2_COINS     1722
#define SOUND_EFFECT_AP_AUDIT_CHUTE_3_COINS     1723
#define SOUND_EFFECT_AP_AUDIT_TOTAL_REPLAYS     1724
#define SOUND_EFFECT_AP_AUDIT_AVG_BALL_TIME     1725
#define SOUND_EFFECT_AP_AUDIT_HISCR_BEAT        1726
#define SOUND_EFFECT_AP_AUDIT_TOTAL_BALLS       1727
#define SOUND_EFFECT_AP_AUDIT_NUM_MATCHES       1728
#define SOUND_EFFECT_AP_AUDIT_MATCH_PERCENTAGE  1729
#define SOUND_EFFECT_AP_AUDIT_LIFETIME_PLAYS    1730
#define SOUND_EFFECT_AP_AUDIT_MINUTES_ON        1731
#define SOUND_EFFECT_AP_AUDIT_CLEAR_AUDITS      1732

#define OM_BASIC_ADJ_IDS_FREEPLAY               0
#define OM_BASIC_ADJ_IDS_BALL_SAVE              1
#define OM_BASIC_ADJ_IDS_TILT_WARNINGS          2
#define OM_BASIC_ADJ_IDS_MUSIC_VOLUME           3
#define OM_BASIC_ADJ_IDS_SOUNDFX_VOLUME         4
#define OM_BASIC_ADJ_IDS_CALLOUTS_VOLUME        5
#define OM_BASIC_ADJ_IDS_BALLS_PER_GAME         6
#define OM_BASIC_ADJ_IDS_TOURNAMENT_MODE        7
#define OM_BASIC_ADJ_IDS_EXTRA_BALL_VALUE       8
#define OM_BASIC_ADJ_IDS_SPECIAL_VALUE          9 
#define OM_BASIC_ADJ_IDS_RESET_DURING_GAME      10
#define OM_BASIC_ADJ_IDS_SCORE_LEVEL_1          11
#define OM_BASIC_ADJ_IDS_SCORE_LEVEL_2          12
#define OM_BASIC_ADJ_IDS_SCORE_LEVEL_3          13
#define OM_BASIC_ADJ_IDS_SCORE_AWARDS           14
#define OM_BASIC_ADJ_IDS_SCROLLING_SCORES       15
#define OM_BASIC_ADJ_IDS_HISCR                  16
#define OM_BASIC_ADJ_IDS_CREDITS                17
#define OM_BASIC_ADJ_IDS_CPC_1                  18
#define OM_BASIC_ADJ_IDS_CPC_2                  19
#define OM_BASIC_ADJ_IDS_CPC_3                  20
#define OM_BASIC_ADJ_IDS_MATCH_FEATURE          21
#define OM_BASIC_ADJ_FINISHED                   22
#define SOUND_EFFECT_AP_FREEPLAY                (1740 + OM_BASIC_ADJ_IDS_FREEPLAY)
#define SOUND_EFFECT_AP_BALL_SAVE_SECONDS       (1740 + OM_BASIC_ADJ_IDS_BALL_SAVE)
#define SOUND_EFFECT_AP_TILT_WARNINGS           (1740 + OM_BASIC_ADJ_IDS_TILT_WARNINGS)
#define SOUND_EFFECT_AP_MUSIC_VOLUME            (1740 + OM_BASIC_ADJ_IDS_MUSIC_VOLUME)
#define SOUND_EFFECT_AP_SOUNDFX_VOLUME          (1740 + OM_BASIC_ADJ_IDS_SOUNDFX_VOLUME)
#define SOUND_EFFECT_AP_CALLOUTS_VOLUME         (1740 + OM_BASIC_ADJ_IDS_CALLOUTS_VOLUME)
#define SOUND_EFFECT_AP_BALLS_PER_GAME          (1740 + OM_BASIC_ADJ_IDS_BALLS_PER_GAME)
#define SOUND_EFFECT_AP_TOURNAMENT_MODE         (1740 + OM_BASIC_ADJ_IDS_TOURNAMENT_MODE)
#define SOUND_EFFECT_AP_EXTRA_BALL_VALUE        (1740 + OM_BASIC_ADJ_IDS_EXTRA_BALL_VALUE)
#define SOUND_EFFECT_AP_SPECIAL_VALUE           (1740 + OM_BASIC_ADJ_IDS_SPECIAL_VALUE)
#define SOUND_EFFECT_AP_RESET_DURING_GAME       (1740 + OM_BASIC_ADJ_IDS_RESET_DURING_GAME)
#define SOUND_EFFECT_AP_ADJ_SCORE_LEVEL_1       (1740 + OM_BASIC_ADJ_IDS_SCORE_LEVEL_1)
#define SOUND_EFFECT_AP_ADJ_SCORE_LEVEL_2       (1740 + OM_BASIC_ADJ_IDS_SCORE_LEVEL_2)
#define SOUND_EFFECT_AP_ADJ_SCORE_LEVEL_3       (1740 + OM_BASIC_ADJ_IDS_SCORE_LEVEL_3)
#define SOUND_EFFECT_AP_SCORE_AWARDS            (1740 + OM_BASIC_ADJ_IDS_SCORE_AWARDS)
#define SOUND_EFFECT_AP_SCROLLING_SCORES        (1740 + OM_BASIC_ADJ_SCROLLING_SCORES)
#define SOUND_EFFECT_AP_ADJ_HISCR               (1740 + OM_BASIC_ADJ_IDS_HISCR)
#define SOUND_EFFECT_AP_ADJ_CREDITS             (1740 + OM_BASIC_ADJ_IDS_CREDITS)
#define SOUND_EFFECT_AP_ADJ_CPC_1               (1740 + OM_BASIC_ADJ_IDS_CPC_1)
#define SOUND_EFFECT_AP_ADJ_CPC_2               (1740 + OM_BASIC_ADJ_IDS_CPC_2)
#define SOUND_EFFECT_AP_ADJ_CPC_3               (1740 + OM_BASIC_ADJ_IDS_CPC_3)
#define SOUND_EFFECT_AP_MATCH_FEATURE           (1740 + OM_BASIC_ADJ_IDS_MATCH_FEATURE)

#define SOUND_EFFECT_OM_EASY_RULES_INSTRUCTIONS           1770
#define SOUND_EFFECT_OM_MEDIUM_RULES_INSTRUCTIONS         1771
#define SOUND_EFFECT_OM_HARD_RULES_INSTRUCTIONS           1772
#define SOUND_EFFECT_OM_PROGRESSIVE_RULES_INSTRUCTIONS    1773
#define SOUND_EFFECT_OM_CUSTOM_RULES_INSTRUCTIONS         1774

#define OM_GAME_ADJ_EASY_DIFFICULTY                 0
#define OM_GAME_ADJ_MEDIUM_DIFFICULTY               1
#define OM_GAME_ADJ_HARD_DIFFICULTY                 2
#define OM_GAME_ADJ_PROGRESSIVE_DIFFICULTY          3
#define OM_GAME_ADJ_CUSTOM_DIFFICULTY               4
#define SOUND_EFFECT_AP_DIFFICULTY                  (1790 + OM_GAME_ADJ_EASY_DIFFICULTY)

// CUSTOMIZATION TODO:
// Create defines for custom parameters and be sure to 
// create the associated callouts (starting at 1800)
#define OM_GAME_ADJ_TROUGH_EJECT_STRENGTH                   0
#define OM_GAME_ADJ_SAUCER_EJECT_STRENGTH                   1
#define OM_GAME_ADJ_FINISHED                                2
#define SOUND_EFFECT_AP_GAME_ADJUSTMENT_CALLOUTS_START      (1800 + OM_GAME_ADJ_TROUGH_EJECT_STRENGTH)

unsigned long SoundSettingTimeout;
unsigned long SoundTestStart;
byte SoundTestSequence;
unsigned short RestoreBackgroundTrack = BACKGROUND_TRACK_NONE;
  
void RunOperatorMenu() {
  if (!Menus.UpdateMenu(CurrentTime)) {
    // Menu is done
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    Audio.StopAllAudio();
    RPU_TurnOffAllLamps();
    if (MachineState==MACHINE_STATE_ATTRACT) {
      RPU_SetDisplayBallInPlay(0, true);
    } else {
      RPU_SetDisplayBallInPlay(CurrentBallInPlay);
    }
    SoundSettingTimeout = 0;
    return;
  }

  // It's up to this function to eject balls if requested
  if (Menus.BallEjectInProgress()) {
    if (CountBallsInTrough()) {
      if (CurrentTime > (LastTimeBallServed+1500)) {
        LastTimeBallServed = CurrentTime;
        RPU_PushToSolenoidStack(SOL_SERVE_BALL, BallServeSolenoidStrength, true);
      }
    }
  } else {
    LastTimeBallServed = 0;
  }
  
  byte topLevel = Menus.GetTopLevel();
  byte subLevel = Menus.GetSubLevel();

  if (Menus.HasTopLevelChanged()) {
    // Play an audio prompt for the top level
    SoundTestStart = 0;
    if (Audio.GetBackgroundSong()!=BACKGROUND_TRACK_NONE) {
      RestoreBackgroundTrack = Audio.GetBackgroundSong();
    }
    Audio.StopAllAudio();
    Audio.PlaySound((unsigned short)topLevel + SOUND_EFFECT_AP_TOP_LEVEL_MENU_ENTRY, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
    if (Menus.GetTopLevel()==OPERATOR_MENU_GAME_RULES_LEVEL) Menus.SetNumSubLevels(4);
    if (Menus.GetTopLevel()==OPERATOR_MENU_BASIC_ADJ_MENU) {
      GetCPCSelection(0); // make sure CPC values have been read
      Menus.SetNumSubLevels(OM_BASIC_ADJ_FINISHED);
    }
    if (Menus.GetTopLevel()==OPERATOR_MENU_GAME_ADJ_MENU) Menus.SetNumSubLevels(OM_GAME_ADJ_FINISHED);
  }
  if (Menus.HasSubLevelChanged()) {
    SoundTestStart = 0;
    // Play an audio prompt for the sub level    
    Audio.StopAllAudio();
    if (topLevel==OPERATOR_MENU_SELF_TEST_MENU) {
      Audio.PlaySound((unsigned short)subLevel + SOUND_EFFECT_AP_TEST_LAMPS, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);

      if (subLevel==OPERATOR_MENU_TEST_SOUNDS) {
        SoundTestStart = CurrentTime + 1000;
        SoundTestSequence = 0;
      } else {
        SoundTestStart = 0;
      }
    } else if (topLevel==OPERATOR_MENU_AUDITS_MENU) {
      unsigned long *currentAdjustmentUL = NULL;
      byte currentAdjustmentStorageByte = 0;
      byte adjustmentType = OPERATOR_MENU_AUD_CLEARABLE;

      switch (subLevel) {
        case 0:
          Audio.PlaySound(SOUND_EFFECT_AP_AUDIT_TOTAL_PLAYS, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
          currentAdjustmentStorageByte = RPU_TOTAL_PLAYS_EEPROM_START_BYTE;
          break;
        case 1:
          Audio.PlaySound(SOUND_EFFECT_AP_AUDIT_CHUTE_1_COINS, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
          currentAdjustmentStorageByte = RPU_CHUTE_1_COINS_START_BYTE;
          break;
        case 2:
          Audio.PlaySound(SOUND_EFFECT_AP_AUDIT_CHUTE_2_COINS, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
          currentAdjustmentStorageByte = RPU_CHUTE_2_COINS_START_BYTE;
          break;
        case 3:
          Audio.PlaySound(SOUND_EFFECT_AP_AUDIT_CHUTE_3_COINS, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
          currentAdjustmentStorageByte = RPU_CHUTE_3_COINS_START_BYTE;
          break;
        case 4:
          Audio.PlaySound(SOUND_EFFECT_AP_AUDIT_TOTAL_REPLAYS, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
          currentAdjustmentStorageByte = RPU_TOTAL_REPLAYS_EEPROM_START_BYTE;
          break;
        case 5:
          Audio.PlaySound(SOUND_EFFECT_AP_AUDIT_HISCR_BEAT, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
          currentAdjustmentStorageByte = RPU_TOTAL_HISCORE_BEATEN_START_BYTE;
          break;
      }

      Menus.SetAuditControls(currentAdjustmentUL, currentAdjustmentStorageByte, adjustmentType);

    } else if (topLevel==OPERATOR_MENU_BASIC_ADJ_MENU) {
      Audio.PlaySound((unsigned short)subLevel + SOUND_EFFECT_AP_FREEPLAY, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);

      byte *currentAdjustmentByte = NULL;
      byte currentAdjustmentStorageByte = 0;
      byte adjustmentValues[8] = {0};
      byte numAdjustmentValues = 2;
      byte adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
      short parameterCallout = 0;
      unsigned long *currentAdjustmentUL = NULL;
      
      adjustmentValues[1] = 1;

      switch(subLevel) {
        case OM_BASIC_ADJ_IDS_FREEPLAY:
          currentAdjustmentByte = (byte *)&FreePlayMode;
          currentAdjustmentStorageByte = EEPROM_FREE_PLAY_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_BALL_SAVE:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_LIST;
          numAdjustmentValues = 5;
          adjustmentValues[1] = 5;
          adjustmentValues[2] = 10;
          adjustmentValues[3] = 15;
          adjustmentValues[4] = 20;
          currentAdjustmentByte = &BallSaveNumSeconds;
          currentAdjustmentStorageByte = EEPROM_BALL_SAVE_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_TILT_WARNINGS:
          adjustmentValues[1] = 2;
          currentAdjustmentByte = &MaxTiltWarnings;
          currentAdjustmentStorageByte = EEPROM_TILT_WARNING_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_MUSIC_VOLUME:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 10;
          currentAdjustmentByte = &MusicVolume;
          currentAdjustmentStorageByte = EEPROM_MUSIC_VOLUME_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_SOUNDFX_VOLUME:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 10;
          currentAdjustmentByte = &SoundEffectsVolume;
          currentAdjustmentStorageByte = EEPROM_SFX_VOLUME_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_CALLOUTS_VOLUME:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 10;
          currentAdjustmentByte = &CalloutsVolume;
          currentAdjustmentStorageByte = EEPROM_CALLOUTS_VOLUME_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_BALLS_PER_GAME:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          numAdjustmentValues = 8;
          adjustmentValues[0] = 3;
          adjustmentValues[1] = 10;
          currentAdjustmentByte = &BallsPerGame;
          currentAdjustmentStorageByte = EEPROM_BALLS_OVERRIDE_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_TOURNAMENT_MODE:
          currentAdjustmentByte = (byte *)&TournamentScoring;
          currentAdjustmentStorageByte = EEPROM_TOURNAMENT_SCORING_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_EXTRA_BALL_VALUE:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_SCORE_WITH_DEFAULT;
          currentAdjustmentUL = &ExtraBallValue;
          currentAdjustmentStorageByte = EEPROM_EXTRA_BALL_SCORE_UL;
          break;
        case OM_BASIC_ADJ_IDS_SPECIAL_VALUE:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_SCORE_WITH_DEFAULT;
          currentAdjustmentUL = &SpecialValue;
          currentAdjustmentStorageByte = EEPROM_SPECIAL_SCORE_UL;
          break;
        case OM_BASIC_ADJ_IDS_RESET_DURING_GAME:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_LIST;
          numAdjustmentValues = 5;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 1;
          adjustmentValues[2] = 2;
          adjustmentValues[3] = 3;
          adjustmentValues[4] = 99;
          currentAdjustmentByte = &TimeRequiredToResetGame;
          currentAdjustmentStorageByte = EEPROM_CRB_HOLD_TIME;
          parameterCallout = SOUND_EFFECT_OM_CRB_VALUES;
          break;
        case OM_BASIC_ADJ_IDS_SCORE_LEVEL_1:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_SCORE_WITH_DEFAULT;
          currentAdjustmentUL = &AwardScores[0];
          currentAdjustmentStorageByte = RPU_AWARD_SCORE_1_EEPROM_START_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_SCORE_LEVEL_2:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_SCORE_WITH_DEFAULT;
          currentAdjustmentUL = &AwardScores[1];
          currentAdjustmentStorageByte = RPU_AWARD_SCORE_2_EEPROM_START_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_SCORE_LEVEL_3:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_SCORE_WITH_DEFAULT;
          currentAdjustmentUL = &AwardScores[2];
          currentAdjustmentStorageByte = RPU_AWARD_SCORE_3_EEPROM_START_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_SCORE_AWARDS:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX_DEFAULT;
          adjustmentValues[1] = 7;
          currentAdjustmentByte = &ScoreAwardReplay;
          currentAdjustmentStorageByte = EEPROM_AWARD_OVERRIDE_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_SCROLLING_SCORES:
          currentAdjustmentByte = (byte *)&ScrollingScores;
          currentAdjustmentStorageByte = EEPROM_SCROLLING_SCORES_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_HISCR:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_SCORE_WITH_DEFAULT;
          currentAdjustmentUL = &HighScore;
          currentAdjustmentStorageByte = RPU_HIGHSCORE_EEPROM_START_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_CREDITS:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 40;
          currentAdjustmentByte = &Credits;
          currentAdjustmentStorageByte = RPU_CREDITS_EEPROM_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_CPC_1:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_CPC;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = (NUM_CPC_PAIRS-1);
          currentAdjustmentByte = &(CPCSelection[0]);
          currentAdjustmentStorageByte = RPU_CPC_CHUTE_1_SELECTION_BYTE;
          parameterCallout = SOUND_EFFECT_OM_CPC_VALUES;
          break;
        case OM_BASIC_ADJ_IDS_CPC_2:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_CPC;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = (NUM_CPC_PAIRS-1);
          currentAdjustmentByte = &(CPCSelection[1]);
          currentAdjustmentStorageByte = RPU_CPC_CHUTE_2_SELECTION_BYTE;
          parameterCallout = SOUND_EFFECT_OM_CPC_VALUES;
          break;
        case OM_BASIC_ADJ_IDS_CPC_3:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_CPC;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = (NUM_CPC_PAIRS-1);
          currentAdjustmentByte = &(CPCSelection[2]);
          currentAdjustmentStorageByte = RPU_CPC_CHUTE_3_SELECTION_BYTE;
          parameterCallout = SOUND_EFFECT_OM_CPC_VALUES;
          break;
        case OM_BASIC_ADJ_IDS_MATCH_FEATURE:
          currentAdjustmentByte = (byte *)&MatchFeature;
          currentAdjustmentStorageByte = EEPROM_MATCH_FEATURE_BYTE;
          break;
      }

      Menus.SetParameterControls(   adjustmentType, numAdjustmentValues, adjustmentValues, parameterCallout,
                                    currentAdjustmentStorageByte, currentAdjustmentByte, currentAdjustmentUL );
    } else if (topLevel==OPERATOR_MENU_GAME_RULES_LEVEL) {
      Audio.PlaySound((unsigned short)subLevel + SOUND_EFFECT_AP_DIFFICULTY, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
      byte *currentAdjustmentByte = &GameRulesSelection;
      byte adjustmentValues[8] = {0};
      adjustmentValues[0] = 0;
      // if one of the below parameters is installed, the "HasParameterChanged" 
      // check below will install Easy / Medium / Hard rules

      switch (subLevel) {
        case 0:
          adjustmentValues[1] = 1;
          break;
        case 1:
          adjustmentValues[1] = 2;
          break;
        case 2:
          adjustmentValues[1] = 3;
          break;
        case 3:
          adjustmentValues[1] = 4;
          break;
      }

      Menus.SetParameterControls(   OPERATOR_MENU_ADJ_TYPE_LIST, 2, adjustmentValues, (short)SOUND_EFFECT_OM_EASY_RULES_INSTRUCTIONS-1,
                                    EEPROM_GAME_RULES_SELECTION, currentAdjustmentByte, NULL );
                  
    } else if (topLevel==OPERATOR_MENU_GAME_ADJ_MENU) {
      Audio.PlaySound((unsigned short)subLevel + SOUND_EFFECT_AP_GAME_ADJUSTMENT_CALLOUTS_START, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);

      byte *currentAdjustmentByte = NULL;
      byte currentAdjustmentStorageByte = 0;
      byte adjustmentValues[8] = {0};
      byte numAdjustmentValues = 2;
      byte adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
      short parameterCallout = 0;
      unsigned long *currentAdjustmentUL = NULL;
      
      adjustmentValues[1] = 1;

      // CUSTOMIZATION TODO:
      // Add adjustment values here for game rules

      switch (subLevel) {
        case OM_GAME_ADJ_TROUGH_EJECT_STRENGTH:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_LIST;
          numAdjustmentValues = 6;
          adjustmentValues[0] = 10;
          adjustmentValues[1] = 20;
          adjustmentValues[2] = 30;
          adjustmentValues[3] = 40;
          adjustmentValues[4] = 50;
          adjustmentValues[5] = 60;
          currentAdjustmentByte = &BallServeSolenoidStrength;
          currentAdjustmentStorageByte = EEPROM_TROUGH_EJECT_STRENGTH;
          break;
        case OM_GAME_ADJ_SAUCER_EJECT_STRENGTH:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 5;
          adjustmentValues[1] = 15;
          currentAdjustmentByte = &SaucerSolenoidStrength;
          currentAdjustmentStorageByte = EEPROM_SAUCER_EJECT_STRENGTH;
          break;
      }
      
      Menus.SetParameterControls(   adjustmentType, numAdjustmentValues, adjustmentValues, parameterCallout,
                                    currentAdjustmentStorageByte, currentAdjustmentByte, currentAdjustmentUL );
    }    
  }

  if (Menus.HasParameterChanged()) {
    short parameterCallout = Menus.GetParameterCallout();
    if (parameterCallout) {
      Audio.StopAllAudio();
      Audio.PlaySound((unsigned short)parameterCallout + Menus.GetParameterID(), AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
    }
    if (Menus.GetTopLevel()==OPERATOR_MENU_GAME_RULES_LEVEL) {
      // Install the new rules level
      if (LoadRuleDefaults(GameRulesSelection)) {
        WriteParameters();
      }
    } else if (Menus.GetTopLevel()==OPERATOR_MENU_BASIC_ADJ_MENU) {
      if (Menus.GetSubLevel()==OM_BASIC_ADJ_IDS_MUSIC_VOLUME) {
        if (SoundSettingTimeout) Audio.StopAllAudio();
        Audio.PlaySound(SOUND_EFFECT_BACKGROUND_SONG_1, AUDIO_PLAY_TYPE_WAV_TRIGGER, MusicVolume);
        Audio.SetMusicVolume(MusicVolume);
        SoundSettingTimeout = CurrentTime + 5000;
      } else if (Menus.GetSubLevel()==OM_BASIC_ADJ_IDS_SOUNDFX_VOLUME) {
        if (SoundSettingTimeout) Audio.StopAllAudio();
        Audio.PlaySound(SOUND_EFFECT_SPINNER, AUDIO_PLAY_TYPE_WAV_TRIGGER, SoundEffectsVolume);
        Audio.SetSoundFXVolume(SoundEffectsVolume);
        SoundSettingTimeout = CurrentTime + 5000;
      } else if (Menus.GetSubLevel()==OM_BASIC_ADJ_IDS_CALLOUTS_VOLUME) {
        if (SoundSettingTimeout) Audio.StopAllAudio();
        Audio.PlaySound(SOUND_EFFECT_VP_SHOOT_AGAIN, AUDIO_PLAY_TYPE_WAV_TRIGGER, CalloutsVolume);
        Audio.SetNotificationsVolume(CalloutsVolume);
        SoundSettingTimeout = CurrentTime + 3000;        
      }
    } else if (Menus.GetTopLevel()==OPERATOR_MENU_GAME_ADJ_MENU) {
    }
  }

  if (SoundSettingTimeout && CurrentTime>SoundSettingTimeout) {
    SoundSettingTimeout = 0;
    Audio.StopAllAudio();
  }

  if (SoundTestStart && CurrentTime>SoundTestStart) {
    if (SoundTestSequence==0) {
      PlayBackgroundSong(SOUND_EFFECT_BACKGROUND_SONG_1);
      SoundTestSequence = 1;
    } else if (SoundTestSequence==1 && CurrentTime>(SoundTestStart+5000)) {
      PlaySoundEffect(SOUND_EFFECT_SPINNER);
      SoundTestSequence = 2;
    } else if (SoundTestSequence==2 && CurrentTime>(SoundTestStart+10000)) {
      Audio.QueuePrioritizedNotification(SOUND_EFFECT_VP_EXTRA_BALL, 0, 10, CurrentTime);
      SoundTestSequence = 3;
    }
  }
  
}



////////////////////////////////////////////////////////////////////////////
//
//  Audio Output functions
//
////////////////////////////////////////////////////////////////////////////
void PlayBackgroundSong(unsigned int songNum) {

  if (MusicVolume == 0) return;

  Audio.PlayBackgroundSong(songNum);

}


unsigned long NextSoundEffectTime = 0;

void PlaySoundEffect(unsigned int soundEffectNum) {

  if (MachineState == MACHINE_STATE_INIT_GAMEPLAY) return;

  // Play digital samples on the WAV trigger (numbered same
  // as SOUND_EFFECT_ defines)
  Audio.PlaySound(soundEffectNum, AUDIO_PLAY_TYPE_WAV_TRIGGER);

  // SOUND_EFFECT_ defines can also be translated into
  // commands for the sound card
  switch (soundEffectNum) {
      /*
          case SOUND_EFFECT_LEFT_SHOOTER_LANE:
            Audio.PlaySoundCardWhenPossible(12, CurrentTime, 0, 500, 7);
            break;
          case SOUND_EFFECT_RETURN_TO_SHOOTER_LANE:
            Audio.PlaySoundCardWhenPossible(22, CurrentTime, 0, 500, 8);
            break;
          case SOUND_EFFECT_SAUCER:
            Audio.PlaySoundCardWhenPossible(14, CurrentTime, 0, 500, 7);
            break;
          case SOUND_EFFECT_DROP_TARGET_HURRY:
            Audio.PlaySoundCardWhenPossible(2, CurrentTime, 0, 45, 3);
            break;
          case SOUND_EFFECT_DROP_TARGET_COMPLETE:
            Audio.PlaySoundCardWhenPossible(9, CurrentTime, 0, 1400, 4);
            Audio.PlaySoundCardWhenPossible(19, CurrentTime, 1500, 10, 4);
            break;
          case SOUND_EFFECT_HOOFBEATS:
            Audio.PlaySoundCardWhenPossible(12, CurrentTime, 0, 100, 10);
            break;
          case SOUND_EFFECT_STOP_BACKGROUND:
            Audio.PlaySoundCardWhenPossible(19, CurrentTime, 0, 10, 10);
            break;
          case SOUND_EFFECT_DROP_TARGET_HIT:
            Audio.PlaySoundCardWhenPossible(7, CurrentTime, 0, 150, 5);
            break;
          case SOUND_EFFECT_SPINNER:
            Audio.PlaySoundCardWhenPossible(6, CurrentTime, 0, 25, 2);
            break;
      */
  }
}


void QueueNotification(unsigned int soundEffectNum, byte priority) {
  if (CalloutsVolume == 0) return;

  // With RPU_OS_HARDWARE_REV 4 and above, the WAV trigger has two-way communication,
  // so it's not necesary to tell it the length of a notification. For support for
  // earlier hardware, you'll need an array of VoicePromptLengths for each prompt
  // played (for queueing and ducking)
  //  Audio.QueuePrioritizedNotification(soundEffectNum, VoicePromptLengths[soundEffectNum-SOUND_EFFECT_VP_VOICE_NOTIFICATIONS_START], priority, CurrentTime);
  Audio.QueuePrioritizedNotification(soundEffectNum, 0, priority, CurrentTime);

}


void AlertPlayerUp() {
  QueueNotification(SOUND_EFFECT_VP_PLAYER_1_UP + CurrentPlayer, 1);
}




////////////////////////////////////////////////////////////////////////////
//
//  Diagnostics Mode
//
////////////////////////////////////////////////////////////////////////////

int RunDiagnosticsMode(int curState, boolean curStateChanged) {

  int returnState = curState;

  if (curStateChanged) {

    /*
        char buf[256];
        boolean errorSeen;

        Serial.write("Testing Volatile RAM at IC13 (0x0000 - 0x0080): writing & reading... ");
        Serial.write("3 ");
        delay(500);
        Serial.write("2 ");
        delay(500);
        Serial.write("1 \n");
        delay(500);
        errorSeen = false;
        for (byte valueCount=0; valueCount<0xFF; valueCount++) {
          for (unsigned short address=0x0000; address<0x0080; address++) {
            RPU_DataWrite(address, valueCount);
          }
          for (unsigned short address=0x0000; address<0x0080; address++) {
            byte readValue = RPU_DataRead(address);
            if (readValue!=valueCount) {
              sprintf(buf, "Write/Read failure at address=0x%04X (expected 0x%02X, read 0x%02X)\n", address, valueCount, readValue);
              Serial.write(buf);
              errorSeen = true;
            }
            if (errorSeen) break;
          }
          if (errorSeen) break;
        }
        if (errorSeen) {
          Serial.write("!!! Error in Volatile RAM\n");
        }

        Serial.write("Testing Volatile RAM at IC16 (0x0080 - 0x0100): writing & reading... ");
        Serial.write("3 ");
        delay(500);
        Serial.write("2 ");
        delay(500);
        Serial.write("1 \n");
        delay(500);
        errorSeen = false;
        for (byte valueCount=0; valueCount<0xFF; valueCount++) {
          for (unsigned short address=0x0080; address<0x0100; address++) {
            RPU_DataWrite(address, valueCount);
          }
          for (unsigned short address=0x0080; address<0x0100; address++) {
            byte readValue = RPU_DataRead(address);
            if (readValue!=valueCount) {
              sprintf(buf, "Write/Read failure at address=0x%04X (expected 0x%02X, read 0x%02X)\n", address, valueCount, readValue);
              Serial.write(buf);
              errorSeen = true;
            }
            if (errorSeen) break;
          }
          if (errorSeen) break;
        }
        if (errorSeen) {
          Serial.write("!!! Error in Volatile RAM\n");
        }

        // Check the CMOS RAM to see if it's operating correctly
        errorSeen = false;
        Serial.write("Testing CMOS RAM: writing & reading... ");
        Serial.write("3 ");
        delay(500);
        Serial.write("2 ");
        delay(500);
        Serial.write("1 \n");
        delay(500);
        for (byte valueCount=0; valueCount<0x10; valueCount++) {
          for (unsigned short address=0x0100; address<0x0200; address++) {
            RPU_DataWrite(address, valueCount);
          }
          for (unsigned short address=0x0100; address<0x0200; address++) {
            byte readValue = RPU_DataRead(address);
            if ((readValue&0x0F)!=valueCount) {
              sprintf(buf, "Write/Read failure at address=0x%04X (expected 0x%02X, read 0x%02X)\n", address, valueCount, (readValue&0x0F));
              Serial.write(buf);
              errorSeen = true;
            }
            if (errorSeen) break;
          }
          if (errorSeen) break;
        }

        if (errorSeen) {
          Serial.write("!!! Error in CMOS RAM\n");
        }


        // Check the ROMs
        Serial.write("CMOS RAM dump... ");
        Serial.write("3 ");
        delay(500);
        Serial.write("2 ");
        delay(500);
        Serial.write("1 \n");
        delay(500);
        for (unsigned short address=0x0100; address<0x0200; address++) {
          if ((address&0x000F)==0x0000) {
            sprintf(buf, "0x%04X:  ", address);
            Serial.write(buf);
          }
      //      RPU_DataWrite(address, address&0xFF);
          sprintf(buf, "0x%02X ", RPU_DataRead(address));
          Serial.write(buf);
          if ((address&0x000F)==0x000F) {
            Serial.write("\n");
          }
        }

    */

    //    RPU_EnableSolenoidStack();
    //    RPU_SetDisableFlippers(false);

  }

  return returnState;
}




////////////////////////////////////////////////////////////////////////////
//
//  Attract Mode
//
////////////////////////////////////////////////////////////////////////////
byte AttractLastHeadMode = 255;
boolean AttractCheckedForTrappedBall;
unsigned long AttractModeStartTime;

////////////////////////////////////////////////////////////////////////////
//
//  Playfield attract light show
//
//    Runs on the playfield lamps (the LEDs) while the machine is waiting in
//    game over. Timing, measured from when attract starts or from the last
//    switch activity while in attract:
//      - dark for 30 seconds (ATTRACT_SHOW_FIRST_DELAY_MS)
//      - show runs for 1 minute (ATTRACT_SHOW_RUN_MS)
//      - dark for 3 minutes (ATTRACT_SHOW_DARK_MS), then the show runs again, and so on
//      - 30 minutes after the idle clock started the playfield stays dark
//        until a switch (coin, credit button, ...) restarts the clock
//    The backbox lamps and displays are not part of the show.
//
//    The lamp table was worked out from the Visual Pinball table layout:
//    angle = clockwise position around the playfield (0 = straight up, 256 = full circle),
//    x/y = left-right / top-bottom position, radius = distance from the middle. All 0-255.
//    Not included: lamp 47 (shoot again) and lamp 55 (credits), which mean something.
//
////////////////////////////////////////////////////////////////////////////
#define ATTRACT_SHOW_FIRST_DELAY_MS     30000UL     // idle time before the first show (30 seconds)
#define ATTRACT_SHOW_RUN_MS             60000UL     // how long each show runs (two passes of the 30-second sequence)
#define ATTRACT_SHOW_DARK_MS           180000UL     // dark time between shows
#define ATTRACT_SHOW_TOTAL_MS         1800000UL     // after this long idle, stay dark
#define ATTRACT_SHOW_SEQUENCE_MS        30000UL     // length of the sequence below

struct AttractShowLamp {
  byte lamp;
  byte angle;
  byte x;
  byte y;
  byte radius;
};

const AttractShowLamp AttractShowLamps[] = {
  {10,   1, 129,   0, 169},   // "3" rollover
  {11,  13, 163,   0, 178},   // "4" rollover
  {12,  24, 199,   1, 202},   // "5" rollover
  {44,  51, 223,  93, 158},   // spinner
  {36,  73, 185, 146,  93},   // bonus 9K
  {35,  79, 181, 155,  90},   // bonus 8K
  {39,  79, 199, 164, 121},   // bonus 20K
  {34,  85, 175, 163,  87},   // bonus 7K
  {38,  86, 192, 177, 117},   // bonus 10K
  {33,  91, 170, 172,  87},   // bonus 6K
  {46,  92, 255, 254, 255},   // right special
  {32,  98, 165, 181,  89},   // bonus 5K
  {31, 103, 160, 190,  93},   // bonus 4K
  {30, 109, 156, 199,  99},   // bonus 3K
  {27, 111, 167, 244, 160},   // 5X
  {29, 113, 151, 207, 106},   // bonus 2K
  {28, 117, 146, 216, 115},   // bonus 1K
  {26, 121, 143, 244, 150},   // 4X
  {25, 132, 118, 245, 149},   // 3X
  {23, 137, 110, 214, 111},   // X
  {24, 142,  91, 245, 158},   // 2X
  {22, 144, 102, 200,  98},   // I
  {21, 152,  95, 187,  88},   // N
  {20, 162,  86, 174,  83},   // E
  {45, 164,   0, 255, 253},   // left special
  {19, 172,  79, 160,  82},   // O
  {18, 182,  71, 147,  87},   // H
  {17, 190,  63, 134,  98},   // P
  {16, 204,  30,  96, 156},   // extra ball when lit
  { 8, 233,  57,   0, 201},   // "1" rollover
  { 9, 244,  93,   0, 177}    // "2" rollover
};
#define NUM_ATTRACT_SHOW_LAMPS (sizeof(AttractShowLamps) / sizeof(AttractShowLamps[0]))

unsigned long AttractShowIdleStart = 0;   // the idle clock: attract entry or last switch activity

void SetAttractShowLampsOff() {
  for (byte i = 0; i < NUM_ATTRACT_SHOW_LAMPS; i++) RPU_SetLampState(AttractShowLamps[i].lamp, 0);
}

// showTime = milliseconds since this show started. The sequence is 30 seconds long:
//   0- 8 s  two comets orbit the playfield clockwise
//   8-14 s  three beams sweep round counter-clockwise
//  14-20 s  a bar bounces from top to bottom
//  20-25 s  rings ripple out from the middle
//  25-30 s  odd/even lamps flicker, then the whole playfield strobes
void RunAttractLightShow(unsigned long showTime) {
  unsigned long t = showTime % ATTRACT_SHOW_SEQUENCE_MS;
  byte stage;
  unsigned long st;   // time within the stage
  if (t < 8000UL) { stage = 0; st = t; }
  else if (t < 14000UL) { stage = 1; st = t - 8000UL; }
  else if (t < 20000UL) { stage = 2; st = t - 14000UL; }
  else if (t < 25000UL) { stage = 3; st = t - 20000UL; }
  else { stage = 4; st = t - 25000UL; }

  // Per-stage values that are the same for every lamp
  byte head = 0;
  int barPos = 0;
  int ringEdge = 0;
  switch (stage) {
    case 0: head = (byte)((st * 17UL) / 100UL); break;              // one lap every 1.5 seconds
    case 1: head = (byte)(0 - (byte)((st * 51UL) / 400UL)); break;  // one lap every 2 seconds, other way
    case 2: {
        int u = (int)(((st % 2000UL) * 511UL) / 2000UL);            // up and back down every 2 seconds
        barPos = (u < 256) ? u : (511 - u);
      } break;
    case 3: ringEdge = (int)(((st % 1250UL) * 310UL) / 1250UL); break;  // a ring every 1.25 seconds
    default: break;
  }

  for (byte i = 0; i < NUM_ATTRACT_SHOW_LAMPS; i++) {
    byte angle = AttractShowLamps[i].angle;
    boolean lampOn = false;

    switch (stage) {
      case 0:
        lampOn = ((byte)(head - angle) < 36) || ((byte)(head + 128 - angle) < 36);
        break;
      case 1:
        lampOn = ((byte)(angle - head) < 22) || ((byte)(angle - head - 85) < 22) || ((byte)(angle - head - 170) < 22);
        break;
      case 2: {
          int dy = (int)AttractShowLamps[i].y - barPos;
          if (dy < 0) dy = -dy;
          lampOn = (dy < 48);
        } break;
      case 3: {
          int radius = AttractShowLamps[i].radius;
          lampOn = (radius <= ringEdge) && (radius > (ringEdge - 60));
        } break;
      default:
        if (st < 3500UL) lampOn = (((st / 110UL) + i) & 1) ? true : false;
        else lampOn = ((st / 140UL) & 1) ? false : true;
        break;
    }

    RPU_SetLampState(AttractShowLamps[i].lamp, lampOn ? 1 : 0);
  }
}

// Called every pass through attract mode: decides whether the show is running or the playfield is dark
void ManageAttractLightShow() {
  unsigned long idleTime = CurrentTime - AttractShowIdleStart;

  if (idleTime >= ATTRACT_SHOW_TOTAL_MS || idleTime < ATTRACT_SHOW_FIRST_DELAY_MS) {
    SetAttractShowLampsOff();
    return;
  }

  unsigned long sinceFirstShow = idleTime - ATTRACT_SHOW_FIRST_DELAY_MS;
  unsigned long cycleTime = sinceFirstShow % (ATTRACT_SHOW_RUN_MS + ATTRACT_SHOW_DARK_MS);

  if (cycleTime < ATTRACT_SHOW_RUN_MS) RunAttractLightShow(cycleTime);
  else SetAttractShowLampsOff();
}


// Sketch version: shown on the credits display for the first 8 seconds after power-up so you can tell which build is loaded
#define SKETCH_VERSION                21
#define SKETCH_VERSION_SHOW_MS        8000
boolean SketchVersionShown = false;

int RunAttractMode(int curState, boolean curStateChanged) {

  int returnState = curState;

  if (curStateChanged) {
    // Some sound cards have a special index
    // for a "sound" that will turn off
    // the current background drone or currently
    // playing sound
    RPU_DisableSolenoidStack();
    RPU_TurnOffAllLamps();
    RPU_SetDisableFlippers(true);
    if (DEBUG_MESSAGES) {
      Serial.write("Entering Attract Mode\n\r");
    }
    AttractLastHeadMode = 0;
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    Display_ClearOverride(0xFF);
    Display_UpdateDisplays(0xFF);
    AttractCheckedForTrappedBall = false;
    AttractModeStartTime = CurrentTime;
    AttractShowIdleStart = CurrentTime;   // playfield stays dark for the first 30 seconds

#ifdef RPU_OS_USE_ACCESSORY_LAMP_BOARD
    TopperALB.StopAnimation();
    TopperALB.PlayAnimation(GI_0_SET_COLOR, 250, 0, 0);
    TopperALB.PlayAnimation(GI_1_SET_COLOR, 150, 27, 0);
#endif
  }

  if (!SketchVersionShown) {
    if (CurrentTime < SKETCH_VERSION_SHOW_MS) {
      RPU_SetDisplayCredits(SKETCH_VERSION, true);
    } else {
      RPU_SetDisplayCredits(Credits, !FreePlayMode);
      SketchVersionShown = true;
    }
  }

  if (CurrentTime > (AttractModeStartTime + 5000) && !AttractCheckedForTrappedBall) {
    AttractCheckedForTrappedBall = true;
    if (DEBUG_MESSAGES) {
      Serial.write("In Attract for 10 seconds - make sure there are no balls trapped\n");
    }

    if (RPU_ReadSingleSwitchState(SW_SAUCER)) {
      RPU_PushToSolenoidStack(SOL_SAUCER, SaucerSolenoidStrength, true);
    }
  }

  //RPU_SetLampState(LAMP_APRON_CREDITS, (FreePlayMode || Credits) ? true : false, 0, 200);

  // Game over lamps. Called every pass so "1 CAN PLAY" follows credits as coins are added.
  SetBackboxStatusLamps(false);
  // Handbook p.5: with scores at zero, the player 1 up lamp flashes in game over.
  // (The high-score phase below switches the player up lamps off again.)
  if (CurrentNumPlayers == 0) RPU_SetLampState(LAMP_HEAD_PLAYER_1_UP, 1, 0, 250);

  // Alternate displays between high score and blank
  if (CurrentTime < 16000) {
    if (AttractLastHeadMode != 1) {
      RPU_SetDisplayCredits(Credits, !FreePlayMode);
      RPU_SetDisplayBallInPlay(0, true);
      Display_ClearOverride(0xFF);
    }
    AttractLastHeadMode = 1;
    Display_UpdateDisplays(0);          // between games only display 1 is used
    RPU_SetLampState(LAMP_HEAD_HIGH_SCORE, 0);
  } else if ((CurrentTime / 8000) % 2 == 0) {

    if (AttractLastHeadMode != 2) {
      Display_SetLastTimeScoreChanged(CurrentTime);
    }
    AttractLastHeadMode = 2;
    Display_UpdateDisplays(0, false, false, false, HighScore);   // high score on display 1 only
    RPU_SetLampState(LAMP_HEAD_HIGH_SCORE, 1);
    for (byte count=0; count<4; count++) {
      RPU_SetLampState(PlayerUpLamps[count], 0);
    }
  } else {
    if (AttractLastHeadMode != 3) {
      if (CurrentTime < 32000) {
        for (int count = 0; count < RPU_NUMBER_OF_PLAYERS_ALLOWED; count++) {
          CurrentScores[count] = 0;
        }
        CurrentNumPlayers = 0;
      }
      Display_SetLastTimeScoreChanged(CurrentTime);
    }

    RPU_SetLampState(LAMP_HEAD_HIGH_SCORE, 0);
    // Last game's score for player 1 on display 1 (zero when there has been no game)
    Display_UpdateDisplays(0, false, false, false, (CurrentNumPlayers > 0) ? CurrentScores[0] : 0);

    AttractLastHeadMode = 3;
  }

  // A last game over a million scrolls its total on display 1 (the same 10-second cycle as in play)
  if (AttractLastHeadMode == 3) ShowMillionsScroll(CurrentNumPlayers > 0 ? 1 : 0);
  else if (MillionsBlinkDark[0]) ShowMillionsScroll(0);

  // Between games displays 2-4 stay blank. They light up again as players are added once a game is started.
  for (byte count = 1; count < 4; count++) RPU_SetDisplayBlank(count, 0x00);

  // Playfield light show (30 seconds idle, show, 3 minutes dark, repeat, dark after 30 minutes)
  ManageAttractLightShow();

  byte switchHit;
  while ( (switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY ) {
    // Any switch activity (coin, credit button, someone at the machine) restarts the idle clock
    AttractShowIdleStart = CurrentTime;
    if (switchHit == SW_CREDIT_RESET) {
      if (AddPlayer(true)) returnState = MACHINE_STATE_INIT_GAMEPLAY;
    }
    if (switchHit == SW_COIN_1 || switchHit == SW_COIN_2 || switchHit == SW_COIN_3 ) {
      AddCoinToAudit(SwitchToChuteNum(switchHit));
      AddCoin(SwitchToChuteNum(switchHit));
    }
    if (switchHit == SW_SELF_TEST_SWITCH) {
      Menus.EnterOperatorMenu();
    }
  }

  // If the user was holding the menu button when the game started
  // then kick the balls
  if (CurrentTime < 4000) {
    if (RPU_ReadSingleSwitchState(SW_SELF_TEST_SWITCH)) {
      if (OperatorSwitchPressStarted==0) {
        OperatorSwitchPressStarted = CurrentTime;
      } else if (CurrentTime > (OperatorSwitchPressStarted+500)) {
        Menus.EnterOperatorMenu();
        Menus.BallEjectInProgress(true);
      }
    } else {
      OperatorSwitchPressStarted = 0;
    }
  }

  return returnState;
}





////////////////////////////////////////////////////////////////////////////
//
//  Game Play functions
//
////////////////////////////////////////////////////////////////////////////
byte CountBits(unsigned short intToBeCounted) {
  byte numBits = 0;

  for (byte count = 0; count < 16; count++) {
    numBits += (intToBeCounted & 0x01);
    intToBeCounted = intToBeCounted >> 1;
  }

  return numBits;
}


void SetGameMode(byte newGameMode) {
  LastGameMode = GameMode;
  GameMode = newGameMode;
  GameModeStartTime = 0;
  GameModeEndTime = 0;

  if (DEBUG_MESSAGES) {
    char buf[128];
    sprintf(buf, "Game Mode = %d\n", newGameMode);
    Serial.write(buf);
  }
}

byte CountBallsInTrough() {

  byte numBalls = RPU_ReadSingleSwitchState(SW_OUTHOLE);

  return numBalls;
}



void AddToBonus(byte bonus) {
  Bonus[CurrentPlayer] += bonus;
  if (Bonus[CurrentPlayer] > MAX_DISPLAY_BONUS) {
    Bonus[CurrentPlayer] = MAX_DISPLAY_BONUS;
/*    
    // Some games award a special when
    // bonus is maxed out
    if (!SpecialCollected) {
      if (SpecialAvailable) {
        AwardSpecial();  
        SpecialAvailable = false;
      } else {
        SpecialAvailable = true;
      }
    }
*/    
  } else {
    BonusChanged = CurrentTime;
  }
}


void IncreaseBonusX() {
  // Phoenix multipliers: 2X, 3X, 4X, 5X
  if (BonusX[CurrentPlayer] < 5) {
    BonusX[CurrentPlayer] += 1;
    BonusXAnimationStart = CurrentTime;
  }
}



unsigned long GameStartNotificationTime = 0;
boolean WaitForBallToReachOuthole = false;
unsigned long UpperBallEjectTime = 0;

int InitGamePlay(boolean curStateChanged) {

  if (curStateChanged) {
    RPU_TurnOffAllLamps();
    SetGeneralIlluminationOn(true);
    GameStartNotificationTime = CurrentTime;
    Audio.StopAllAudio();
    QueueNotification(SOUND_EFFECT_VP_ADD_PLAYER_1, 10);
    for (byte count = 0; count < RPU_NUMBER_OF_PLAYERS_ALLOWED; count++) RPU_SetDisplayBlank(count, 0x00);
    RPU_SetDisplayCredits(0, false);
    RPU_SetDisplayBallInPlay(1, true);
    NumberOfBallsLocked = 0;

    // CUSTOMIZATION TODO:
    // Add any game-start parameters here    
  }

  boolean showBIP = (CurrentTime / 100) % 2;
  RPU_SetDisplayBallInPlay(1, showBIP ? true : false);

  if (RPU_ReadSingleSwitchState(SW_SAUCER)) {
    RPU_PushToSolenoidStack(SOL_SAUCER, SaucerSolenoidStrength, true);
  }

  // The start button has been hit only once to get
  // us into this mode, so we assume a 1-player game
  // at the moment
  RPU_SetCoinLockout((Credits >= MaximumCredits) ? true : false);

  // Reset displays & game state variables
  for (int count = 0; count < RPU_NUMBER_OF_PLAYERS_ALLOWED; count++) {
    // Initialize game-specific variables
    Bonus[count] = 0;
    BonusX[count] = 1;
    CurrentAchievements[count] = 0;
    CurrentScores[count] = 0;
    ExtraBallAvailable[count] = 0;
    ExtraBallsBanked[count] = 0;
    ExtraBallsAwarded[count] = 0;
    MillionsCount[count] = 0;
    MillionsBlinkDark[count] = false;
    MillionsScrollStart[count] = 0;
    PhoenixLetters[count] = 0;
    PhoenixSets[count] = 0;
    PhoenixCompletions[count] = 0;
    PhoenixCompleteTime[count] = 0;
    RolloversLit[count] = 0;
    PhoenixXLit[count] = false;
    OutlaneSpecialReady[count] = false;
    SpinnerProgress[count] = 0;
  }

  SamePlayerShootsAgain = false;
  CurrentBallInPlay = 1;
  CurrentNumPlayers = 1;
  CurrentPlayer = 0;
  NumberOfBallsInPlay = 0;
  LastTimePopHit = 0;
  LastTiltWarningTime = 0;
  Display_ClearOverride(0xFF);
  Display_UpdateDisplays(0xFF);
  RPU_EnableSolenoidStack();

  return MACHINE_STATE_INIT_NEW_BALL;
}



int InitNewBall(bool curStateChanged) {

  // If we're coming into this mode for the first time
  // then we have to do everything to set up the new ball
  if (curStateChanged) {

    if (GameRulesSelection==GAME_RULES_PROGRESSIVE) {
      // if the operator has installed "progressive" rules, 
      // then we change the rules based on player number.
      // If we're 1 player, it's EASY
      // First player is EASY, last player is HARD,
      // and in-between players are MEDIUM
      byte rulesLevel = GAME_RULES_EASY;
      if (CurrentPlayer==0) rulesLevel = GAME_RULES_EASY;
      else if (CurrentPlayer==(CurrentNumPlayers-1)) rulesLevel = GAME_RULES_HARD;
      else rulesLevel = GAME_RULES_MEDIUM;
      LoadRuleDefaults(rulesLevel);
      if (CurrentBallInPlay==1) QueueNotification(SOUND_EFFECT_VP_GAME_RULES_EASY + (rulesLevel-GAME_RULES_EASY), 10);
    }

    RPU_TurnOffAllLamps();
    BallFirstSwitchHitTime = 0;

    // The bonus countdown at the end of every ball switches the solenoid stack off. Without this line it stayed
    // off from ball 2 on, so pushes made without the override flag (the drop-bank resets) were silently dropped.
    RPU_EnableSolenoidStack();

    // Turn the flippers on for this ball (attract mode, tilt and game over turn them off).
    // Without this call the flipper-enable line stays off for the whole game.
    RPU_SetDisableFlippers(false);

    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    if (CurrentNumPlayers > 1 && (CurrentBallInPlay != 1 || CurrentPlayer != 0) && !SamePlayerShootsAgain) AlertPlayerUp();
    SamePlayerShootsAgain = false;

    RPU_SetDisplayBallInPlay(CurrentBallInPlay);
    RPU_SetLampState(LAMP_HEAD_TILT, 0);
    for (byte count = 0; count < 4; count++) {
      if (count==CurrentPlayer) RPU_SetLampState(PlayerUpLamps[count], 1, 0, 250);
      else if (count<CurrentNumPlayers) RPU_SetLampState(PlayerUpLamps[count], 1);
      else RPU_SetLampState(PlayerUpLamps[count], 0);
      RPU_SetDisplayBlank(count, 0);
    }

    if (BallSaveNumSeconds > 0) {
      RPU_SetLampState(LAMP_SHOOT_AGAIN, 1, 0, 500);
    }

    BallSaveUsed = false;
    BallTimeInTrough = 0;
    NumTiltWarnings = 0;

    // RPU_TurnOffAllLamps() above cleared the backbox status lamps, so relight them
    SetBackboxStatusLamps(true);

    // Initialize game-specific start-of-ball lights & variables
    GameModeStartTime = 0;
    GameModeEndTime = 0;
    GameMode = GAME_MODE_SKILL_SHOT;

    SpecialCollected = false;
    SpecialAvailable = false;

    PopBumperProgress = 0;
    PlayfieldMultiplier = 1;
    PlayfieldMultiplierTimeLeft = 0;
    BonusXAnimationStart = 0;
    Bonus[CurrentPlayer] = 1;
    BonusX[CurrentPlayer] = 1;
    Display_ResetDisplayTrackingVariables();
    LastTimePopHit = 0;

    SetBallSave(0);

    LastSpinnerHitTime = 0;
    LastTimeSaucerSeen = 0;

    // CUSTOMIZATION TODO:
    // Add any ball start parameters here    

    // Reset Drop Targets
    LeftDropTargets.ResetDropTargets(CurrentTime + 100, true, true);
    RightDropTargets.ResetDropTargets(CurrentTime + 100 + DROP_BANK_RESET_STAGGER_MS, true, true);
    for (byte bankIndex = 0; bankIndex < 2; bankIndex++) {
      BankAllDownSince[bankIndex] = 0;
      BankLastResetRequest[bankIndex] = CurrentTime;   // the backstop leaves the banks alone while these resets happen
      BankBackstopTries[bankIndex] = 0;
    }
    // The centre single drop targets are not reset at the start of a ball (handbook p.5);
    // they are reset by the "2" and "4" rollovers and the inside rollovers.

    RPU_PushToTimedSolenoidStack(SOL_SERVE_BALL, BallServeSolenoidStrength, CurrentTime + 1000, true);
    LastTimeBallServed = CurrentTime + 1000;
    
    NumberOfBallsInPlay = 1;

    PlayBackgroundSong(SOUND_EFFECT_RALLY_MUSIC_1 + (CurrentBallInPlay-1));
  }

  return MACHINE_STATE_NORMAL_GAMEPLAY;
  
  LastTimeThroughLoop = CurrentTime;
}




unsigned long SaucerClosedStart = 0;

void CheckSaucerForStuckBall() {
  if (RPU_ReadSingleSwitchState(SW_SAUCER)) {
    LastTimeSaucerSeen = CurrentTime;
    if (SaucerClosedStart == 0) {
      SaucerClosedStart = CurrentTime;
    } else {
      if (CurrentTime > (SaucerClosedStart + 1500)) {
        RPU_PushToSolenoidStack(SOL_SAUCER, SaucerSolenoidStrength, true);
      }
    }
  } else {
    SaucerClosedStart = 0;
  }

}

void UpdateDropTargets() {
  LeftDropTargets.Update(CurrentTime);
  RightDropTargets.Update(CurrentTime);
}

// Asks a bank (0 = left, 1 = right) to stand back up after delayMs, and remembers when we asked.
void RequestDropBankReset(byte bankIndex, unsigned long delayMs) {
  DropTargetBank &bank = bankIndex ? RightDropTargets : LeftDropTargets;
  BankLastResetRequest[bankIndex] = CurrentTime;
  bank.ResetDropTargets(CurrentTime + delayMs, true);
}

// Backstop for the banks: trusts the switches, not the hit count. If all four targets in a bank are physically
// down and stay down, stand the bank up (no points: whatever was missed was never scored).
void CheckDropBankBackstop() {
  if (NumTiltWarnings > MaxTiltWarnings) return;   // coils are off during a tilt
  for (byte bankIndex = 0; bankIndex < 2; bankIndex++) {
    DropTargetBank &bank = bankIndex ? RightDropTargets : LeftDropTargets;
    if (bank.GetStatus(true) != 0x0F) {
      BankAllDownSince[bankIndex] = 0;
      continue;
    }
    if (BankAllDownSince[bankIndex] == 0) BankAllDownSince[bankIndex] = CurrentTime;
    if ((CurrentTime - BankAllDownSince[bankIndex]) < DROP_BANK_BACKSTOP_STABLE_MS) continue;
    if (BankLastResetRequest[bankIndex] && (CurrentTime - BankLastResetRequest[bankIndex]) < DROP_BANK_BACKSTOP_RETRY_MS) continue;
    if (BankBackstopTries[bankIndex] >= DROP_BANK_BACKSTOP_MAX_TRIES) continue;
    BankBackstopTries[bankIndex] += 1;
    if (DEBUG_MESSAGES) Serial.write(bankIndex ? "Right bank all down: backstop reset\n" : "Left bank all down: backstop reset\n");
    RequestDropBankReset(bankIndex, 50);
  }
}



byte GameModeStage;
boolean DisplaysNeedRefreshing = false;
unsigned long LastTimePromptPlayed = 0;
unsigned long LastTimeAwardsChecked = 0;
unsigned long LastTimeJackpotAdjusted = 0;
unsigned long LastLoopTick = 0;



// CUSTOMIZATION TODO:
// Put your game logic here.
// This function manages all timers, flags, and lights
int ManageGameMode() {
  int returnState = MACHINE_STATE_NORMAL_GAMEPLAY;

  boolean specialAnimationRunning = false;
  boolean statusRunning = false;

  UpdateDropTargets();
  CheckDropBankBackstop();

  if ((CurrentTime - LastSwitchHitTime) > 3000) TimersPaused = true;
  else TimersPaused = false;

  CheckSaucerForStuckBall();

  switch ( GameMode ) {
    case GAME_MODE_SKILL_SHOT:
      // Phoenix has no skill shot: this mode just waits for the ball to be plunged
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = 0;
        GameModeStage = 0;
        SetGeneralIlluminationOn(true);
      }

      if (BallFirstSwitchHitTime != 0) {
        Display_ClearOverride(0xFF);
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;

    case GAME_MODE_UNSTRUCTURED_PLAY:
      // If this is the first time in this mode
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        DisplaysNeedRefreshing = true;
        if (DEBUG_MESSAGES) {
          Serial.write("Entering unstructured play\n");
        }
        SetGeneralIlluminationOn(true);
        unsigned short songNum = SOUND_EFFECT_BACKGROUND_SONG_1;
        if (CurrentBallInPlay>1 && CurrentBallInPlay<5) songNum = SOUND_EFFECT_BACKGROUND_SONG_2 + (CurrentTime%3);
        if (CurrentBallInPlay==BallsPerGame) songNum = SOUND_EFFECT_BACKGROUND_SONG_5;
        if (Audio.GetBackgroundSong() != songNum) {
          PlayBackgroundSong(songNum);
        }
        GameModeStage = 0;
        LastTimePromptPlayed = 0;
      }

      // Display Overrides in Unstructured Play
      if (LastSpinnerHitTime) {

        if (CurrentTime > (LastSpinnerHitTime + 5000)) {
          LastSpinnerHitTime = 0;
        } else {
          if (SpinnerProgress[CurrentPlayer] && SpinnerProgress[CurrentPlayer] < 100) {
            int spinnerHitsRemaining = 100 - SpinnerProgress[CurrentPlayer];
            Display_OverrideScoreDisplay(CurrentPlayer, spinnerHitsRemaining, DISPLAY_OVERRIDE_ANIMATION_CENTER);
          }
        }
        DisplaysNeedRefreshing = true;
      } else if (PlayfieldMultiplier > 1) {
        // Playfield X value is only reset during unstructured play
        if (PlayfieldMultiplierTimeLeft && (CurrentTime > LastLoopTick)) {
          unsigned long numTicks = CurrentTime - LastLoopTick;
          if (numTicks > PlayfieldMultiplierTimeLeft) {
            PlayfieldMultiplierTimeLeft = 0;
            PlayfieldMultiplier = 1;
          } else {
            PlayfieldMultiplierTimeLeft -= numTicks;
          }
        } else {
          for (byte count = 0; count < RPU_NUMBER_OF_PLAYERS_ALLOWED; count++) {
            if (count != CurrentPlayer && !statusRunning) Display_OverrideScoreDisplay(count, PlayfieldMultiplier, DISPLAY_OVERRIDE_SYMMETRIC_BOUNCE);
          }
          DisplaysNeedRefreshing = true;
        }
      } else if (DisplaysNeedRefreshing) {
        DisplaysNeedRefreshing = false;
        Display_ClearOverride(0xFF);
      }

      break;

  }

  if ( !statusRunning && !specialAnimationRunning && NumTiltWarnings <= MaxTiltWarnings ) {
    // CUSTOMIZATION TODO:
    // add in any custom lamp functions here
    ShowBonusLamps();
    ShowShootAgainLamp();
    ShowPhoenixLamps();
  }
  ShowPlayerLamps();

  if (Display_UpdateDisplays(0xFF, false, (BallFirstSwitchHitTime == 0) ? true : false, (BallFirstSwitchHitTime > 0 && ((CurrentTime - Display_GetLastTimeScoreChanged()) > 2000)) ? true : false)) {
    Audio.StopSound(SOUND_EFFECT_SCORE_TICK);
    PlaySoundEffect(SOUND_EFFECT_SCORE_TICK);
  }
  ShowMillionsScroll(CurrentNumPlayers);   // players over a million scroll their whole score every 10 seconds

  // Check to see if ball is in the outhole
  if (CountBallsInTrough() > (TotalBallsLoaded - (NumberOfBallsInPlay + NumberOfBallsLocked))) {

    if (BallTimeInTrough == 0) {
      // If this is the first time we're seeing too many balls in the trough, we'll wait to make sure
      // everything is settled
      BallTimeInTrough = CurrentTime;
    } else {

      // Make sure the ball stays on the sensor for at least
      // 0.5 seconds to be sure that it's not bouncing or passing through
      if ((CurrentTime - BallTimeInTrough) > 750) {

        if ((BallFirstSwitchHitTime == 0 && NumTiltWarnings <= MaxTiltWarnings)) {
          // Nothing hit yet, so return the ball to the player
          RPU_PushToTimedSolenoidStack(SOL_SERVE_BALL, BallServeSolenoidStrength, CurrentTime);
          BallTimeInTrough = 0;
          returnState = MACHINE_STATE_NORMAL_GAMEPLAY;
        } else {
          // if we haven't used the ball save, and we're under the time limit, then save the ball
          if (BallSaveEndTime && CurrentTime < (BallSaveEndTime + BALL_SAVE_GRACE_PERIOD)) {
            RPU_PushToTimedSolenoidStack(SOL_SERVE_BALL, BallServeSolenoidStrength, CurrentTime + 100);

            RPU_SetLampState(LAMP_SHOOT_AGAIN, 0);
            BallTimeInTrough = CurrentTime;
            returnState = MACHINE_STATE_NORMAL_GAMEPLAY;

            if (NumberOfBallSavesRemaining && NumberOfBallSavesRemaining != 0xFF) {
              NumberOfBallSavesRemaining -= 1;
              if (NumberOfBallSavesRemaining == 0) {
                BallSaveEndTime = 0;
                if (DEBUG_MESSAGES) {
                  Serial.write("Last ball save\n");
                }
                QueueNotification(SOUND_EFFECT_VP_BALL_SAVE, 10);
              } else {
                if (DEBUG_MESSAGES) {
                  Serial.write("Not last ball save\n");
                }                
              }
            }

          } else {

            NumberOfBallsInPlay -= 1;
            if (NumberOfBallsInPlay == 0) {
              Display_ClearOverride(0xFF);
              Audio.StopAllAudio();
              //PlaySoundEffect(SOUND_EFFECT_BALL_OVER);
              returnState = MACHINE_STATE_COUNTDOWN_BONUS;
            }
          }
        }
      }
    }
  } else {
    BallTimeInTrough = 0;
  }

  LastLoopTick = CurrentTime;
  LastTimeThroughLoop = CurrentTime;
  return returnState;
}



unsigned long CountdownStartTime = 0;
unsigned long LastCountdownReportTime = 0;
unsigned long BonusCountDownEndTime = 0;
byte DecrementingBonusCounter;
byte IncrementingBonusXCounter;
byte TotalBonus = 0;
byte TotalBonusX = 0;
byte BonusSoundIncrement;
boolean CountdownBonusHurryUp = false;
int LastBonusSoundPlayed = 0;

int CountDownDelayTimes[] = {130, 130, 130, 100, 100, 60, 60, 60, 60, 60};

int CountdownBonus(boolean curStateChanged) {

  // If this is the first time through the countdown loop
  if (curStateChanged) {
    
    // Turn off solenoids
    RPU_DisableSolenoidStack();

    if (RPU_ReadSingleSwitchState(SW_SAUCER)) {
      RPU_PushToSolenoidStack(SOL_SAUCER, SaucerSolenoidStrength, true);
    }

    CountdownStartTime = CurrentTime;
    LastCountdownReportTime = CurrentTime;
    ShowBonusLamps();
    IncrementingBonusXCounter = 1;
    DecrementingBonusCounter = Bonus[CurrentPlayer];
    TotalBonus = Bonus[CurrentPlayer];
    TotalBonusX = BonusX[CurrentPlayer];
    CountdownBonusHurryUp = false;
    BonusSoundIncrement = 0;
    LastBonusSoundPlayed = 0;

    BonusCountDownEndTime = 0xFFFFFFFF;
    // Some sound cards have a special index
    // for a "sound" that will turn off
    // the current background drone or currently
    // playing sound
    //    PlaySoundEffect(SOUND_EFFECT_STOP_BACKGROUND);
  }

  // CUSTOMIZATION TODO:
  // If your game has flipper switches, you can use them to 
  // speed up the bonus countdown
  //if (RPU_ReadSingleSwitchState(SW_LEFT_FLIPPER) && RPU_ReadSingleSwitchState(SW_RIGHT_FLIPPER)) CountdownBonusHurryUp = true;

  unsigned long countdownDelayTime = (unsigned long)(CountDownDelayTimes[IncrementingBonusXCounter - 1]);
  if (CountdownBonusHurryUp && countdownDelayTime > ((unsigned long)CountDownDelayTimes[9])) countdownDelayTime = CountDownDelayTimes[9];

  if ((CurrentTime - LastCountdownReportTime) > countdownDelayTime) {

    if (DecrementingBonusCounter) {

      // Only give sound & score if this isn't a tilt
      if (NumTiltWarnings <= MaxTiltWarnings) {
        int soundToPlay = SOUND_EFFECT_BONUS_1 + (BonusSoundIncrement/4);
        if (soundToPlay>SOUND_EFFECT_BONUS_8) soundToPlay = SOUND_EFFECT_BONUS_8;
        if (LastBonusSoundPlayed!=0) Audio.StopSound(LastBonusSoundPlayed);
        PlaySoundEffect(soundToPlay);
        LastBonusSoundPlayed = soundToPlay;
        BonusSoundIncrement += 1;
        CurrentScores[CurrentPlayer] += 1000;
        PlayScoreSounds(1000);   // original 1,000 sound for each bonus step
      }

      DecrementingBonusCounter -= 1;
      Bonus[CurrentPlayer] = DecrementingBonusCounter;
      ShowBonusLamps();

    } else if (BonusCountDownEndTime == 0xFFFFFFFF) {
      IncrementingBonusXCounter += 1;
      if (BonusX[CurrentPlayer] > 1) {
        DecrementingBonusCounter = TotalBonus;
        Bonus[CurrentPlayer] = TotalBonus;
        ShowBonusLamps();
        BonusX[CurrentPlayer] -= 1;
        if (BonusX[CurrentPlayer] == 9) BonusX[CurrentPlayer] = 8;
      } else {
        BonusX[CurrentPlayer] = TotalBonusX;
        Bonus[CurrentPlayer] = TotalBonus;
        BonusCountDownEndTime = CurrentTime + 1000;
      }
    }
    LastCountdownReportTime = CurrentTime;
  }

  if (CurrentTime > BonusCountDownEndTime) {

    if (DEBUG_MESSAGES) {
      Serial.write("Count down over, moving to ball over\n");
    }
    // Reset any lights & variables of goals that weren't completed
    BonusCountDownEndTime = 0xFFFFFFFF;
    return MACHINE_STATE_BALL_OVER;
  }

  return MACHINE_STATE_COUNTDOWN_BONUS;
}



void CheckHighScores() {
  unsigned long highestScore = 0;
  int highScorePlayerNum = 0;
  for (int count = 0; count < CurrentNumPlayers; count++) {
    unsigned long totalScore = CurrentScores[count] + ((unsigned long)MillionsCount[count] * SCORE_WRAP_VALUE);
    if (totalScore > highestScore) {
      highestScore = totalScore;
      highScorePlayerNum = count;
    }
  }

  if (highestScore > HighScore) {
    HighScore = highestScore;
    if (HighScoreReplay) {
      AddCredit(false, 3);
      RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE) + 3);
    }
    RPU_WriteULToEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, highestScore);
    RPU_WriteULToEEProm(RPU_TOTAL_HISCORE_BEATEN_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_HISCORE_BEATEN_START_BYTE) + 1);

    for (int count = 0; count < RPU_NUMBER_OF_PLAYERS_ALLOWED; count++) {
      if (count == highScorePlayerNum) {
        RPU_SetDisplay(count, CurrentScores[count], true, 2);
      } else {
        RPU_SetDisplayBlank(count, 0x00);
      }
    }

    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, KNOCKER_SOLENOID_STRENGTH, CurrentTime, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, KNOCKER_SOLENOID_STRENGTH, CurrentTime + 300, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, KNOCKER_SOLENOID_STRENGTH, CurrentTime + 600, true);
  }
}


unsigned long MatchSequenceStartTime = 0;
unsigned long MatchDelay = 150;
byte MatchDigit = 0;
byte NumMatchSpins = 0;
byte ScoreMatches = 0;

int ShowMatchSequence(boolean curStateChanged) {
  if (!MatchFeature) return MACHINE_STATE_ATTRACT;

  if (curStateChanged) {
    MatchSequenceStartTime = CurrentTime;
    MatchDelay = 1500;
    MatchDigit = CurrentTime % 10;
    NumMatchSpins = 0;
    RPU_SetLampState(LAMP_HEAD_MATCH, 1, 0);
    RPU_SetDisableFlippers();
    ScoreMatches = 0;
  }

  if (NumMatchSpins < 40) {

    if (NumMatchSpins<10) {
      byte blankMask = (0x3F << ((NumMatchSpins)/2)) & 0x3F;
      RPU_SetDisplayBlank(0, RPU_GetDisplayBlank(0) & blankMask);
      RPU_SetDisplayBlank(1, RPU_GetDisplayBlank(1) & blankMask);      
    } else if (NumMatchSpins>16 && NumMatchSpins<40) {
      if (CurrentNumPlayers>2) {
        unsigned long scoreMatch1 = CurrentScores[0] % 100;
        unsigned long scoreMatch2 = CurrentScores[1] % 100;
        unsigned long scoreMatch3 = CurrentScores[2] % 100;
        unsigned long scoreMatch4 = CurrentScores[3] % 100;
        unsigned long display1, display2;
        byte display1Mask, display2Mask;
        byte shiftPhase = (NumMatchSpins - 16)/3;
        switch (shiftPhase) {
          case 0:
            display1 = 10*scoreMatch1;
            display1Mask = 0x18;
            display2 = 10*scoreMatch2;
            display2Mask = 0x18;
            break;
          case 1:
            display1 = 100*scoreMatch1;
            display1Mask = 0x0C;
            display2 = 100*scoreMatch2;
            display2Mask = 0x0C;
            break;
          case 2:
            display1 = 1000*scoreMatch1;
            display1Mask = 0x06;
            display2 = 1000*scoreMatch2 + scoreMatch3/10;
            display2Mask = 0x26;
            break;
          case 3:
            display1 = 10000*scoreMatch1;
            display1Mask = 0x03;
            display2 = 10000*scoreMatch2 + scoreMatch3;
            display2Mask = 0x33;
            break;
          case 4:
            display1 = 10000*scoreMatch1;
            display1Mask = 0x03;
            display2 = scoreMatch3 * 10;
            display2Mask = 0x18;
            break;
          case 5:
            display1 = 10000*scoreMatch1;
            display1Mask = 0x03;
            display2 = 100*scoreMatch3;
            display2Mask = 0x0C;
            break;
          case 6:
            display1 = 10000*scoreMatch1 + scoreMatch2/10;
            display1Mask = 0x23;
            display2 = 1000*scoreMatch3;
            display2Mask = 0x06;
            if (CurrentNumPlayers==4) {
              display2 += scoreMatch4 / 10;
              display2Mask = 0x26;
            }
            break;
          case 7:
            display1 = 10000*scoreMatch1 + scoreMatch2;
            display1Mask = 0x33;
            display2 = 10000*scoreMatch3;
            display2Mask = 0x03;
            if (CurrentNumPlayers==4) {
              display2 += scoreMatch4;
              display2Mask = 0x33;
            }            
            break;
        }
        RPU_SetDisplayBlank(0, display1Mask);
        RPU_SetDisplayBlank(1, display2Mask);
        RPU_SetDisplay(0, display1);
        RPU_SetDisplay(1, display2);        
      }
    }
    
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      MatchDigit += 1;
      if (MatchDigit > 9) MatchDigit = 0;
      //PlaySoundEffect(10+(MatchDigit%2));
      PlaySoundEffect(SOUND_EFFECT_MATCH_SPIN);
      RPU_SetDisplayBallInPlay((int)MatchDigit * 10);
      MatchDelay += 50 + 4 * NumMatchSpins;
      NumMatchSpins += 1;
      RPU_SetLampState(LAMP_HEAD_MATCH, NumMatchSpins % 2, 0);

      if (NumMatchSpins == 40) {
        RPU_SetLampState(LAMP_HEAD_MATCH, 0);
        MatchDelay = CurrentTime - MatchSequenceStartTime;
      }
    }
  }

  if (NumMatchSpins >= 40 && NumMatchSpins <= 43) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      if ( (CurrentNumPlayers > (NumMatchSpins - 40)) && ((CurrentScores[NumMatchSpins - 40] / 10) % 10) == MatchDigit) {
        ScoreMatches |= (1 << (NumMatchSpins - 40));
        AddSpecialCredit();
        MatchDelay += 1000;
        NumMatchSpins += 1;
        RPU_SetLampState(LAMP_HEAD_MATCH, 1);
      } else {
        NumMatchSpins += 1;
      }
      if (NumMatchSpins == 44) {
        MatchDelay += 5000;
      }
    }
  }

  if (NumMatchSpins > 43) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      return MACHINE_STATE_ATTRACT;
    }
  }

  for (int count = 0; count < RPU_NUMBER_OF_PLAYERS_ALLOWED; count++) {
    if (CurrentNumPlayers<3) {
      if ((ScoreMatches >> count) & 0x01) {
        // If this score matches, we're going to flash the last two digits
        byte upperMask = 0x0F;
        byte lowerMask = 0x30;
        if (RPU_OS_NUM_DIGITS == 7) {
          upperMask = 0x1F;
          lowerMask = 0x60;
        }
        if ( (CurrentTime / 200) % 2 ) {
          RPU_SetDisplayBlank(count, RPU_GetDisplayBlank(count) & upperMask);
        } else {
          RPU_SetDisplayBlank(count, RPU_GetDisplayBlank(count) | lowerMask);
        }
      }
    } else {
      // With more than 2 players, the scores will be compressed
      if ((ScoreMatches >> count) & 0x01) {
        byte displayMask0 = 0, displayMask1 = 0;
        if (RPU_OS_NUM_DIGITS == 7) {
          switch (count) {
            case 0: displayMask0 = 0x03; break;
            case 1: displayMask0 = 0x60; break;
            case 2: displayMask1 = 0x03; break;
            case 3: displayMask1 = 0x60; break;
          }
        } else {
          switch (count) {
            case 0: displayMask0 = 0x03; break;
            case 1: displayMask0 = 0x30; break;
            case 2: displayMask1 = 0x03; break;
            case 3: displayMask1 = 0x30; break;
          }
        }
        
        if ( (CurrentTime / 200) % 2 ) {
          RPU_SetDisplayBlank(0, RPU_GetDisplayBlank(0) & ~displayMask0);
          RPU_SetDisplayBlank(1, RPU_GetDisplayBlank(1) & ~displayMask1);
        } else {
          RPU_SetDisplayBlank(0, RPU_GetDisplayBlank(0) | displayMask0);
          RPU_SetDisplayBlank(1, RPU_GetDisplayBlank(1) | displayMask1);
        }
      }
    }
  }

  return MACHINE_STATE_MATCH_MODE;
}




////////////////////////////////////////////////////////////////////////////
//
//  Switch Handling functions
//
////////////////////////////////////////////////////////////////////////////
/*
  // Example lock function

  void HandleLockSwitch(byte lockIndex) {

  if (GameMode==GAME_MODE_UNSTRUCTURED_PLAY) {
    // If this player has a lock available
    if (PlayerLockStatus[CurrentPlayer] & (LOCK_1_AVAILABLE<<lockIndex)) {
      // Lock the ball
      LockBall(lockIndex);
      SetGameMode(GAME_MODE_OFFER_LOCK);
    } else {
      if ((MachineLocks & (LOCK_1_ENGAGED<<lockIndex))==0) {
        // Kick unlocked ball
        RPU_PushToSolenoidStack(SOL_UPPER_BALL_EJECT, 12, true);
      }
    }
  }
  }

*/

int HandleSystemSwitches(int curState, byte switchHit) {
  int returnState = curState;
  switch (switchHit) {
    case SW_SELF_TEST_SWITCH:
      Menus.EnterOperatorMenu();
      break;
    case SW_COIN_1:
    case SW_COIN_2:
    case SW_COIN_3:
      AddCoinToAudit(SwitchToChuteNum(switchHit));
      AddCoin(SwitchToChuteNum(switchHit));
      break;
    case SW_CREDIT_RESET:
      if (MachineState == MACHINE_STATE_MATCH_MODE) {
        // If the first ball is over, pressing start again resets the game
        if (Credits >= 1 || FreePlayMode) {
          if (!FreePlayMode) {
            Credits -= 1;
            RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
            RPU_SetDisplayCredits(Credits, !FreePlayMode);
          }
          returnState = MACHINE_STATE_INIT_GAMEPLAY;
        }
      } else {
        CreditResetPressStarted = CurrentTime;
      }
      break;
    case SW_OUTHOLE:
      // Some machines have a kicker to move the ball
      // from the outhole to the re-shooter ramp
      break;
    case SW_SLAM:
      // Super slam tilt: zero every score and return to game over (handbook p.5)
      for (byte count = 0; count < RPU_NUMBER_OF_PLAYERS_ALLOWED; count++) CurrentScores[count] = 0;
      Audio.StopAllAudio();
      returnState = MACHINE_STATE_ATTRACT;
      break;
    case SW_ROLL_TILT:
    case SW_PLAYFIELD_TILT:
    case SW_PLUMB_TILT:
      if (BallFirstSwitchHitTime) {
        // Only the plumb bob has warnings; ball roll and playfield tilts tilt immediately (handbook p.5)
        if ( switchHit != SW_PLUMB_TILT || CurrentTime > (LastTiltWarningTime + TILT_WARNING_DEBOUNCE_TIME) ) {
          LastTiltWarningTime = CurrentTime;
          if (switchHit == SW_PLUMB_TILT) NumTiltWarnings += 1;
          else NumTiltWarnings = MaxTiltWarnings + 1;
          if (NumTiltWarnings > MaxTiltWarnings) {
            SetGeneralIlluminationOn(false);
            RPU_DisableSolenoidStack();
            RPU_SetDisableFlippers(true);
            RPU_TurnOffAllLamps();
            RPU_SetLampState(LAMP_HEAD_TILT, 1);
            SetBackboxStatusLamps(true);   // a tilt doesn't end the game: keep BALL IN PLAY and CAN PLAY lit
            Audio.StopAllAudio();
            if (BallSaveEndTime) {
              BallSaveEndTime = 0;
              NumberOfBallSavesRemaining = 0;
            }
            PlaySoundEffect(SOUND_EFFECT_TILT);
          } else {
            PlaySoundEffect(SOUND_EFFECT_TILT_WARNING);
          }
        }
      } else {
        // Tilt before ball is plunged -- show a timer in ManageGameMode if desired
        if ( CurrentTime > (LastTiltWarningTime + TILT_WARNING_DEBOUNCE_TIME) ) {
          PlaySoundEffect(SOUND_EFFECT_TILT_WARNING);
        }
        LastTiltWarningTime = CurrentTime;
      }
      break;
  }

  return returnState;
}







// A drop target went down before the ball reached a 1-5 rollover: count it so the bank still knows how many are
// down, but score nothing and leave the ball unplayed. If it was the 4th, stand the bank back up.
void CountDropBankHitQuietly(DropTargetBank &bank, byte switchHit) {
  byte bankIndex = (&bank == &RightDropTargets) ? 1 : 0;
  bank.HandleDropTargetHit(switchHit);
  if (bank.CheckIfBankCleared()) RequestDropBankReset(bankIndex, DROP_BANK_COMPLETE_RESET_DELAY_MS);
}

void HandleDropBankHit(DropTargetBank &bank, byte switchHit) {
  byte bankIndex = (&bank == &RightDropTargets) ? 1 : 0;
  bank.HandleDropTargetHit(switchHit);
  CurrentScores[CurrentPlayer] += SCORE_DROP_TARGET * PlayfieldMultiplier;

  if (bank.CheckIfBankCleared()) {
    // All four down: advance the 2X-5X multiplier and the bonus, then reset the bank
    RequestDropBankReset(bankIndex, DROP_BANK_COMPLETE_RESET_DELAY_MS);
    CurrentScores[CurrentPlayer] += SCORE_DROP_BANK_COMPLETE * PlayfieldMultiplier;
    IncreaseBonusX();
    AddToBonus(1);
    PlaySoundEffect(SOUND_EFFECT_DROP_TARGET_COMPLETE);
  } else {
    PlaySoundEffect(SOUND_EFFECT_DROP_TARGET_SOUND_1);
  }
}

void HandleLeftDropTarget(byte switchHit) {
  HandleDropBankHit(LeftDropTargets, switchHit);
}


void HandleRightDropTarget(byte switchHit) {
  HandleDropBankHit(RightDropTargets, switchHit);
}





void HandleSpinnerProgress() {
  CurrentScores[CurrentPlayer] += (SpinnerIsLit() ? SCORE_SPINNER_LIT : SCORE_SPINNER) * PlayfieldMultiplier;
}


void ValidateAndRegisterPlayfieldSwitch() {
  LastSwitchHitTime = CurrentTime;
  if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
}


// CUSTOMIZATION TODO:
// This is where switch handling happens
void HandleGamePlaySwitches(byte switchHit) {

  // Ball 1 only: nothing scores until the ball has been plunged and reaches one of the "1"-"5" rollovers at the
  // top or the spinner (a ball can miss the rollovers and go down the ramp to the spinner). Until then the ball
  // does not count as played (a ball that comes back to the trough is served again), so switch noise while the
  // ball is fed or the drop targets reset cannot score or start the ball. From ball 2 on there is no such wait.
  if (CurrentBallInPlay == 1 && BallFirstSwitchHitTime == 0 &&
      !(switchHit >= SW_ROLLOVER_1 && switchHit <= SW_ROLLOVER_5) && switchHit != SW_SPINNER) {
    // Drop targets that go down this early (the right bank is next to the shooter lane) still have to be counted,
    // otherwise the bank never sees its 4th target and stays down. No points, no bonus, ball still unplayed.
    if (switchHit >= SW_DROP_L_1 && switchHit <= SW_DROP_L_4) CountDropBankHitQuietly(LeftDropTargets, switchHit);
    else if (switchHit >= SW_DROP_R_1 && switchHit <= SW_DROP_R_4) CountDropBankHitQuietly(RightDropTargets, switchHit);
    return;
  }

  switch (switchHit) {

    // Jet bumpers: left and right 10, centre 100
    case SW_JET_LEFT:
    case SW_JET_RIGHT:
      CurrentScores[CurrentPlayer] += SCORE_JET_SIDE * PlayfieldMultiplier;
      PlaySoundEffect(SOUND_EFFECT_POP_BUMPER);
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_JET_CENTER:
      CurrentScores[CurrentPlayer] += SCORE_JET_CENTER * PlayfieldMultiplier;
      PlaySoundEffect(SOUND_EFFECT_POP_BUMPER);
      ValidateAndRegisterPlayfieldSwitch();
      break;

    // Kickers: 10 each
    case SW_KICKER_LEFT:
      CurrentScores[CurrentPlayer] += SCORE_KICKER * PlayfieldMultiplier;
      LightNumber(1);                                       // playfield: left kicker "LITES 2"
      PlaySoundEffect(SOUND_EFFECT_LEFT_SLING);
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_KICKER_RIGHT:
      CurrentScores[CurrentPlayer] += SCORE_KICKER * PlayfieldMultiplier;
      LightNumber(3);                                       // playfield: right kicker "LITES 4"
      PlaySoundEffect(SOUND_EFFECT_RIGHT_SLING);
      ValidateAndRegisterPlayfieldSwitch();
      break;

    // Standups: 10 on the drop bank assemblies, 50 on the others
    case SW_LEFT_BANK_STANDUP:
    case SW_RIGHT_BANK_STANDUP:
      CurrentScores[CurrentPlayer] += SCORE_BANK_STANDUP * PlayfieldMultiplier;
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_LEFT_STANDUP:
    case SW_TOP_RIGHT_STANDUP:
    case SW_BOTTOM_LEFT_STANDUP:
    case SW_BOTTOM_RIGHT_STANDUP:
      CurrentScores[CurrentPlayer] += SCORE_STANDUP * PlayfieldMultiplier;
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_BULLSEYE_TARGET:
      HandleBullseyeHit();
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_LEFT_TARGET:
    case SW_RIGHT_TARGET:
      CurrentScores[CurrentPlayer] += SCORE_SIDE_TARGET * PlayfieldMultiplier;
      LightNumber(2);                                       // playfield: upper side targets "LITES 3"
      ValidateAndRegisterPlayfieldSwitch();
      break;

    // "1" through "5" rollovers
    case SW_ROLLOVER_1:
    case SW_ROLLOVER_2:
    case SW_ROLLOVER_3:
    case SW_ROLLOVER_4:
    case SW_ROLLOVER_5:
      HandleNumberRollover(switchHit - SW_ROLLOVER_1);
      ValidateAndRegisterPlayfieldSwitch();
      break;

    // Inside rollovers reset the matching centre drop target
    case SW_LEFT_INSIDE_ROLLOVER:
      ResetCenterDropTarget(true);
      CurrentScores[CurrentPlayer] += SCORE_INSIDE_ROLLOVER * PlayfieldMultiplier;
      AddToBonus(1);
      LightNumber(1);                                       // same as the top "2" rollover
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_RIGHT_INSIDE_ROLLOVER:
      ResetCenterDropTarget(false);
      CurrentScores[CurrentPlayer] += SCORE_INSIDE_ROLLOVER * PlayfieldMultiplier;
      AddToBonus(1);
      LightNumber(3);                                       // same as the top "4" rollover
      ValidateAndRegisterPlayfieldSwitch();
      break;

    // Outside rollovers: alternating special once "I" is lit
    case SW_LEFT_OUTSIDE_ROLLOVER:
      HandleOutlaneSpecial(true);
      CurrentScores[CurrentPlayer] += SCORE_OUTSIDE_ROLLOVER * PlayfieldMultiplier;
      AddToBonus(1);
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_RIGHT_OUTSIDE_ROLLOVER:
      HandleOutlaneSpecial(false);
      CurrentScores[CurrentPlayer] += SCORE_OUTSIDE_ROLLOVER * PlayfieldMultiplier;
      AddToBonus(1);
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_DROP_L_1:
    case SW_DROP_L_2:
    case SW_DROP_L_3:
    case SW_DROP_L_4:
      HandleLeftDropTarget(switchHit);
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_DROP_R_1:
    case SW_DROP_R_2:
    case SW_DROP_R_3:
    case SW_DROP_R_4:
      HandleRightDropTarget(switchHit);
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_DROP_CENTER_LEFT:
    case SW_DROP_CENTER_RIGHT:
      CurrentScores[CurrentPlayer] += SCORE_CENTER_DROP * PlayfieldMultiplier;
      AddToBonus(1);
      ValidateAndRegisterPlayfieldSwitch();
      break;

    case SW_SPINNER:
      HandleSpinnerProgress();
      ValidateAndRegisterPlayfieldSwitch();
      break;

  }

}



////////////////////////////////////////////////////////////////////////////
//
//  Original Phoenix sounds
//
//    The Phoenix sound board is driven by four coil outputs, one for each
//    scoring digit: 10, 100, 1,000 and 10,000 points (test-02 coils 9-12).
//    Whenever the score goes up, the matching coil is pulsed once for each
//    unit scored: 3,000 points = three pulses of the 1,000 sound.
//    This works with no WAV Trigger installed.
//
////////////////////////////////////////////////////////////////////////////
#define SCORE_SOUND_STRENGTH          2       // pulse strength for the sound coils
#define SCORE_SOUND_SPACING_MS        90      // time between pulses of one sound
#define SCORE_SOUND_MAX_PULSES        5       // most pulses of one sound for a single score change
#define SCORE_SOUND_ENABLED           true    // set false to silence the original sounds

void PlayScoreSounds(unsigned long pointsScored) {
  if (!SCORE_SOUND_ENABLED || pointsScored == 0) return;

  const byte soundCoils[4] = { SOL_SOUND_10000, SOL_SOUND_1000, SOL_SOUND_100, SOL_SOUND_10 };
  const unsigned long unitValues[4] = { 10000, 1000, 100, 10 };
  unsigned long remaining = pointsScored;
  // Higher sounds first; the lower ones follow on top of each other's timing
  for (byte place = 0; place < 4; place++) {
    unsigned long count = remaining / unitValues[place];
    remaining = remaining % unitValues[place];
    if (count > SCORE_SOUND_MAX_PULSES) count = SCORE_SOUND_MAX_PULSES;
    for (unsigned long pulse = 0; pulse < count; pulse++) {
      RPU_PushToTimedSolenoidStack(soundCoils[place], SCORE_SOUND_STRENGTH, CurrentTime + pulse * SCORE_SOUND_SPACING_MS, true);
    }
  }
}

// Game over tune on the original sound board: a descending run of the four chimes, then the low one again
#define GAME_OVER_TUNE_ENABLED        true
#define GAME_OVER_TUNE_NOTE_MS        220     // time between notes

void PlayGameOverTune() {
  if (!GAME_OVER_TUNE_ENABLED) return;
  const byte tune[5] = { SOL_SOUND_10000, SOL_SOUND_1000, SOL_SOUND_100, SOL_SOUND_10, SOL_SOUND_10000 };
  for (byte note = 0; note < 5; note++) {
    unsigned long when = CurrentTime + 200 + (unsigned long)note * GAME_OVER_TUNE_NOTE_MS;
    if (note == 4) when += GAME_OVER_TUNE_NOTE_MS;   // short pause before the last note
    RPU_PushToTimedSolenoidStack(tune[note], SCORE_SOUND_STRENGTH, when, true);
  }
}

int RunGamePlayMode(int curState, boolean curStateChanged) {
  int returnState = curState;
  unsigned long scoreAtTop = CurrentScores[CurrentPlayer];

  // Very first time into gameplay loop
  if (curState == MACHINE_STATE_INIT_GAMEPLAY) {
    returnState = InitGamePlay(curStateChanged);
  } else if (curState == MACHINE_STATE_INIT_NEW_BALL) {
    returnState = InitNewBall(curStateChanged);
  } else if (curState == MACHINE_STATE_NORMAL_GAMEPLAY) {
    returnState = ManageGameMode();
  } else if (curState == MACHINE_STATE_COUNTDOWN_BONUS) {
    Display_ClearOverride(0xFF);
    Display_UpdateDisplays(0xFF, true);
    returnState = CountdownBonus(curStateChanged);
  } else if (curState == MACHINE_STATE_BALL_OVER) {
    RPU_SetDisplayCredits(Credits, !FreePlayMode);

    // Spend one banked extra ball when the ball ends
    if (ExtraBallsBanked[CurrentPlayer] > 0) {
      ExtraBallsBanked[CurrentPlayer] -= 1;
      SamePlayerShootsAgain = true;
      ShootAgainCycleStart = CurrentTime;
    }

    if (SamePlayerShootsAgain) {
      QueueNotification(SOUND_EFFECT_VP_SHOOT_AGAIN, 10);
      returnState = MACHINE_STATE_INIT_NEW_BALL;
    } else {

      CurrentPlayer += 1;
      if (CurrentPlayer >= CurrentNumPlayers) {
        CurrentPlayer = 0;
        CurrentBallInPlay += 1;
      }

      scoreAtTop = CurrentScores[CurrentPlayer];

      if (CurrentBallInPlay > BallsPerGame) {
        RPU_SetDisableFlippers(true);   // game over: flippers off
        CheckHighScores();
        PlaySoundEffect(SOUND_EFFECT_GAME_OVER);
        PlayGameOverTune();   // original sound board: game over tune
        for (int count = 0; count < CurrentNumPlayers; count++) {
          RPU_SetDisplay(count, CurrentScores[count], true, 2);
        }

        for (byte count = 0; count < 4; count++) {
          RPU_SetLampState(PlayerUpLamps[count], 0);
        }

        // Game over: GAME OVER lamp on, BALL IN PLAY off (the match digits replace the ball number)
        SetBackboxStatusLamps(false);

        if (MatchEnabled) {
          returnState = MACHINE_STATE_MATCH_MODE;
        } else {
          returnState = MACHINE_STATE_ATTRACT;
        }
      }
      else returnState = MACHINE_STATE_INIT_NEW_BALL;
    }
  } else if (curState == MACHINE_STATE_MATCH_MODE) {
    returnState = ShowMatchSequence(curStateChanged);
  }

  byte switchHit;
  unsigned long lastBallFirstSwitchHitTime = BallFirstSwitchHitTime;

  while ( (switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY ) {
    returnState = HandleSystemSwitches(curState, switchHit);
    if (NumTiltWarnings <= MaxTiltWarnings) HandleGamePlaySwitches(switchHit);
  }

  if (CreditResetPressStarted) {
    if (CurrentBallInPlay < 2) {
      // If we haven't finished the first ball, each press adds a player (up to 4);
      // once all four are in, further presses add credits (up to 8)
      if (!AddPlayer()) AddCredit(false, 1);
      if (DEBUG_MESSAGES) {
        Serial.write("Start game button pressed\n\r");
      }
      CreditResetPressStarted = 0;
    } else {
      if (RPU_ReadSingleSwitchState(SW_CREDIT_RESET)) {
        if (TimeRequiredToResetGame != 99 && (CurrentTime - CreditResetPressStarted) >= ((unsigned long)TimeRequiredToResetGame * 1000)) {
          // If the first ball is over, pressing start again resets the game
          if (Credits >= 1 || FreePlayMode) {
            if (!FreePlayMode) {
              Credits -= 1;
              RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
              RPU_SetDisplayCredits(Credits, !FreePlayMode);
            }
            returnState = MACHINE_STATE_INIT_GAMEPLAY;
            CreditResetPressStarted = 0;
          }
        }
      } else {
        CreditResetPressStarted = 0;
      }
    }

  }

  if (lastBallFirstSwitchHitTime == 0 && BallFirstSwitchHitTime != 0) {
    BallSaveEndTime = BallFirstSwitchHitTime + ((unsigned long)BallSaveNumSeconds) * 1000;
    NumberOfBallSavesRemaining = 1;
  }
  if (CurrentTime > (BallSaveEndTime + BALL_SAVE_GRACE_PERIOD)) {
    BallSaveEndTime = 0;
    NumberOfBallSavesRemaining = 0;
  }

  // Wrap at 1,000,000 and count the million (no credit; the card's credits are the replay scores)
  while (CurrentScores[CurrentPlayer] >= SCORE_WRAP_VALUE) {
    CurrentScores[CurrentPlayer] -= SCORE_WRAP_VALUE;
    if (MillionsCount[CurrentPlayer] < 250) MillionsCount[CurrentPlayer] += 1;
    MillionsScrollStart[CurrentPlayer] = CurrentTime;   // scroll the new total right away
  }

  if (scoreAtTop != CurrentScores[CurrentPlayer]) {
    Display_SetLastTimeScoreChanged(CurrentTime);
    // Original Phoenix sounds: one pulse per unit scored (only when the score went up)
    if (CurrentScores[CurrentPlayer] > scoreAtTop) PlayScoreSounds(CurrentScores[CurrentPlayer] - scoreAtTop);
    if (!TournamentScoring) {
      for (int awardCount = 0; awardCount < 3; awardCount++) {
        if (AwardScores[awardCount] != 0 && scoreAtTop < AwardScores[awardCount] && CurrentScores[CurrentPlayer] >= AwardScores[awardCount]) {
          // Player has just passed an award score, so we need to award it
          if (((ScoreAwardReplay >> awardCount) & 0x01)) {
            AddSpecialCredit();
          } else {
            AwardExtraBall(true);
          }
        }
      }
    }

  }

  return returnState;
}


#if (RPU_MPU_ARCHITECTURE>=10)
unsigned long LastLEDUpdateTime = 0;
byte LEDPhase = 0;
#endif

#ifdef DEBUG_SHOW_LOOPS_PER_SECOND
unsigned long NumLoops = 0;
unsigned long LastLoopReportTime = 0;
#endif

void loop() {

  CurrentTime = millis();
  int newMachineState = MachineState;
  
#ifdef DEBUG_SHOW_LOOPS_PER_SECOND
  NumLoops += 1;
  if (LastLoopReportTime==0) LastLoopReportTime = CurrentTime;
  if (CurrentTime>(LastLoopReportTime+1000)) {
    LastLoopReportTime = CurrentTime;
    char buf[128];
    sprintf(buf, "Loop running at %lu Hz\n", NumLoops);
    Serial.write(buf);
    NumLoops = 0;
  }
#endif

  if (Menus.OperatorMenusActive()) {
    RunOperatorMenu();
  } else {
    if (RestoreBackgroundTrack!=BACKGROUND_TRACK_NONE) {
      PlayBackgroundSong(RestoreBackgroundTrack);
      RestoreBackgroundTrack = BACKGROUND_TRACK_NONE;
    }
    if (MachineState < 0) {
      newMachineState = 0;
    } else if (MachineState == MACHINE_STATE_ATTRACT) {
      newMachineState = RunAttractMode(MachineState, MachineStateChanged);
    } else if (MachineState == MACHINE_STATE_DIAGNOSTICS) {
      newMachineState = RunDiagnosticsMode(MachineState, MachineStateChanged);
    } else {
      newMachineState = RunGamePlayMode(MachineState, MachineStateChanged);
    }  
  }
  
  if (newMachineState != MachineState) {
    MachineState = newMachineState;
    MachineStateChanged = true;
  } else {
    MachineStateChanged = false;
  }

  RPU_Update(CurrentTime);
  Audio.Update(CurrentTime);

#if (RPU_MPU_ARCHITECTURE>=10)
  if (LastLEDUpdateTime == 0 || (CurrentTime - LastLEDUpdateTime) > 250) {
    LastLEDUpdateTime = CurrentTime;
    RPU_SetBoardLEDs((LEDPhase % 8) == 1 || (LEDPhase % 8) == 3, (LEDPhase % 8) == 5 || (LEDPhase % 8) == 7);
    LEDPhase += 1;
  }
#endif

}
