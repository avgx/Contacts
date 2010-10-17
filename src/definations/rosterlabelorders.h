#ifndef DEF_ROSTERLABELORDERS_H
#define DEF_ROSTERLABELORDERS_H

//�������������� ����������� �������
#define RLID_FOOTER_TEXT                          -5
#define RLID_DISPLAY                              -4
#define RLID_DECORATION                           -3
#define RLID_INDICATORBRANCH                      -2
#define RLID_NULL                                 -1

//����� ������������� ����� �� ������
#define RLAP_LEFT_CENTER                          0
//����� ������������� �����
#define RLAP_LEFT_TOP                             10000
//����� ������������� ������
#define RLAP_RIGHT_TOP                            20000
//����� ������������� ������ �� ������
#define RLAP_RIGHT_CENTER                         30000
//����� ������������� �� ������ ������
#define RLAP_CENTER_TOP                           40000
//����� ������������� �� ������
#define RLAP_CENTER_CENTER                        50000

/*************************************************************************/
#define RLO_DECORATION                            RLAP_LEFT_CENTER + 500

#define RLO_CONNECTING                            RLAP_LEFT_TOP + 100
#define RLO_DISPLAY                               RLAP_LEFT_TOP + 500

#define RLO_PRIVACY                               RLAP_RIGHT_TOP + 300
#define RLO_CONNECTION_ENCRYPTED                  RLAP_RIGHT_TOP + 500

#define RLO_AVATAR_IMAGE                          RLAP_RIGHT_CENTER + 500

#endif
