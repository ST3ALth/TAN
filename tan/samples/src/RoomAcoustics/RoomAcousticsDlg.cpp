//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

// RoomAcousticsDlg.cpp : implementation file
//

#include "stdafx.h"

//#include "wav.h"
#include "RoomAcoustics.h"
#include "RoomAcousticsDlg.h"
//#include "..\TrueAudioLib\trueaudiobase.h"
//#include "..\TrueAudioVR\trueaudiovr.h"
//#include "AMDTrueAudioLib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    enum { IDD = IDD_ABOUTBOX };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CRoomAcousticsDlg dialog




CRoomAcousticsDlg::CRoomAcousticsDlg(
    AmdTrueAudioVR *pTAVR, 
    TANConverterPtr pConverter, 
    CWnd* pParent /*=NULL*/
) : 
    CDialog(CRoomAcousticsDlg::IDD, pParent),
    m_pTAVR(pTAVR),
    m_spConverter(pConverter),
	src1EnableMic(0),
	src1TrackHeadPos(0),
	src1MuteDirectPath(0),
	headX(0.0f), headY(0.0f), headZ(0.0f),
	yaw(0.0f), pitch(0.0f), roll(0.0f),
    earSpacing(0.16f),
    autoSpinHead(0),
    roomWidth(5.f), roomHeight(5.f), roomLength(5.f),
	roomDampLeft(1.f), roomDampRight(1.f), roomDampFront(1.f), roomDampBack(1.f), roomDampTop(1.f), roomDampBottom(1.f),
    convolutionLength(1024),
	bufferSize(256),
	roomDevIdx(0),
	convDevIdx(0),
	sourceIdx(0),
	nFiles(1)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
 

	for (int i = 0; i < MAX_SOURCES; i++){
		srcX[i] = srcY[i] = srcZ[i] = 0.0;
		srcEnable[i] = 0;
	}
	srcEnable[0] = 1;

	for (int i = 0; i < MAX_DEVICES; i++){
        deviceNames[i] = new char[MAX_PATH + 2];
        memset(deviceNames[i], 0, (MAX_PATH + 2));
    }

	for (int idx = 0; idx < MAX_SOURCES; idx++){
        waveFileNames[idx] = new char[MAX_PATH + 2];
		waveFileNames[idx][0] = '\0';
	}
}

CRoomAcousticsDlg::~CRoomAcousticsDlg()
{		
    for (int i = 0; i < MAX_DEVICES; i++){
        if (deviceNames[i])
        {
            delete[] deviceNames[i];
            deviceNames[i] = NULL;
        }
    }
    for (int idx = 0; idx < MAX_SOURCES; idx++){
		delete[] waveFileNames[idx];
	}
}


void CRoomAcousticsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CRoomAcousticsDlg, CDialog)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BUTTONOPEN, &CRoomAcousticsDlg::OnBnClickedButtonopen)
    ON_EN_KILLFOCUS(IDC_EDITFILE, &CRoomAcousticsDlg::OnEnKillfocusEditfile)
    ON_EN_CHANGE(IDC_EDIT_HEAD_X, &CRoomAcousticsDlg::OnEnChangeEditHeadX)
    ON_EN_CHANGE(IDC_EDIT_HEAD_Y, &CRoomAcousticsDlg::OnEnChangeEditHeadY)
    ON_EN_CHANGE(IDC_EDIT_HEAD_Z, &CRoomAcousticsDlg::OnEnChangeEditHeadZ)
    ON_EN_CHANGE(IDC_EDIT_EARSPACING, &CRoomAcousticsDlg::OnEnChangeEditEarspacing)
    ON_EN_CHANGE(IDC_EDIT_ROOM_WIDTH, &CRoomAcousticsDlg::OnEnChangeEditRoomWidth)
    ON_EN_CHANGE(IDC_EDIT_ROOM_HEIGHT, &CRoomAcousticsDlg::OnEnChangeEditRoomHeight)
    ON_EN_CHANGE(IDC_EDIT_ROOM_LENGTH, &CRoomAcousticsDlg::OnEnChangeEditRoomLength)
    ON_EN_CHANGE(IDC_EDIT_ROOM_DAMP_LEFT, &CRoomAcousticsDlg::OnEnChangeEditRoomDampLeft)
    ON_EN_CHANGE(IDC_EDIT_ROOM_DAMP_FRONT, &CRoomAcousticsDlg::OnEnChangeEditRoomDampFront)
    ON_EN_CHANGE(IDC_EDIT_ROOM_DAMP_TOP, &CRoomAcousticsDlg::OnEnChangeEditRoomDampTop)
    ON_EN_CHANGE(IDC_EDIT_ROOM_DAMP_RIGHT, &CRoomAcousticsDlg::OnEnChangeEditRoomDampRight)
    ON_EN_CHANGE(IDC_EDIT_ROOM_DAMP_BACK, &CRoomAcousticsDlg::OnEnChangeEditRoomDampBack)
    ON_EN_CHANGE(IDC_EDIT_ROOM_DAMP_BOTTOM, &CRoomAcousticsDlg::OnEnChangeEditRoomDampBottom)
    ON_EN_CHANGE(IDC_EDIT_CONV_LENGTH, &CRoomAcousticsDlg::OnEnChangeEditConvLength)
    ON_EN_CHANGE(IDC_EDIT_BUFFER_SIZE, &CRoomAcousticsDlg::OnEnChangeEditBufferSize)
    ON_BN_CLICKED(IDC_RADIO_GPU, &CRoomAcousticsDlg::OnBnClickedRadioGpu)
    ON_BN_CLICKED(IDC_RADIO_CPU, &CRoomAcousticsDlg::OnBnClickedRadioCpu)
    ON_BN_CLICKED(IDC_LOAD, &CRoomAcousticsDlg::OnBnClickedLoad)
    ON_BN_CLICKED(IDC_SAVE, &CRoomAcousticsDlg::OnBnClickedSave)
    ON_BN_CLICKED(IDC_EXPORT, &CRoomAcousticsDlg::OnBnClickedExport)
    ON_BN_CLICKED(IDC_CHECK_EN_SRC, &CRoomAcousticsDlg::OnBnClickedCheckEnSrc)
    ON_EN_CHANGE(IDC_EDIT_SRC_X, &CRoomAcousticsDlg::OnEnChangeEditSrcX)
    ON_EN_CHANGE(IDC_EDIT_SRC_Y, &CRoomAcousticsDlg::OnEnChangeEditSrcY)
    ON_EN_CHANGE(IDC_EDIT_SRC_Z, &CRoomAcousticsDlg::OnEnChangeEditSrcZ)
    ON_BN_CLICKED(IDC_CHECK_SRC_TRACK_HEAD, &CRoomAcousticsDlg::OnBnClickedCheckSrc1TrackHead)
    ON_BN_CLICKED(IDC_CHECK_SRC_MUTE_DIRECT, &CRoomAcousticsDlg::OnBnClickedCheckSrc1MuteDirect)
    ON_EN_KILLFOCUS(IDC_EDITFILE, &CRoomAcousticsDlg::OnEnKillfocusEditfile2)
    ON_BN_CLICKED(IDC_CHECK_EN_SRCMIC, &CRoomAcousticsDlg::OnBnClickedCheckEnSrc1mic)
    ON_BN_CLICKED(IDC_BUTTONOPEN, &CRoomAcousticsDlg::OnBnClickedButtonopen2)
    ON_BN_CLICKED(IDOK, &CRoomAcousticsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO_RG_CPU, &CRoomAcousticsDlg::OnBnClickedRadioRgCpu)
	ON_BN_CLICKED(IDC_RADIO_RG_GPU, &CRoomAcousticsDlg::OnBnClickedRadioRgGpu)
	ON_BN_CLICKED(IDC_HEADSPIN, &CRoomAcousticsDlg::OnBnClickedHeadspin)
	ON_LBN_SELCHANGE(IDC_LIST_DEVICES_ROOM, &CRoomAcousticsDlg::OnLbnSelchangeListDevicesRoom)
	ON_LBN_SELCHANGE(IDC_LIST_DEVICES_CONV, &CRoomAcousticsDlg::OnLbnSelchangeListDevicesConv)
	ON_LBN_SELCHANGE(IDC_LIST_SOURCES, &CRoomAcousticsDlg::OnLbnSelchangeListSources)
	ON_EN_CHANGE(IDC_EDIT_NFILES, &CRoomAcousticsDlg::OnEnChangeEditNfiles)
END_MESSAGE_MAP()



void CRoomAcousticsDlg::updateAllFields()
{
    char buffer[80];
    char fmt[20] = "%7.3f";
    //sprintf_s(buffer, fmt, srcX[0]);
    //SetDlgItemText(IDC_EDIT_SRC1_X, buffer);
    //sprintf_s(buffer, fmt, srcY[0]);
    //SetDlgItemText(IDC_EDIT_SRC1_Y, buffer);
    //sprintf_s(buffer, fmt, srcZ[0]);
    //SetDlgItemText(IDC_EDIT_SRC1_Z, buffer);

	CheckDlgButton(IDC_CHECK_EN_SRC, srcEnable[sourceIdx]);
    CheckDlgButton(IDC_CHECK_EN_SRCMIC, src1EnableMic);
    CheckDlgButton(IDC_CHECK_SRC_TRACK_HEAD, src1TrackHeadPos);
    CheckDlgButton(IDC_CHECK_SRC_MUTE_DIRECT, src1MuteDirectPath);

	CWnd* pWnd = NULL;
	pWnd = GetDlgItem(IDC_LIST_SOURCES);
	for (int i = 0; i < MAX_SOURCES; i++) {
		char buf[20];
		itoa(i + 1, buf, 10);
		pWnd->SendMessage(LB_ADDSTRING, 0, (LPARAM)buf);
	}
	pWnd->SendMessage(LB_SETCURSEL, sourceIdx);

    if (src1TrackHeadPos) {
        CWnd* pWnd = GetDlgItem(IDC_EDIT_SRC_X);
        pWnd->EnableWindow(FALSE);
        pWnd = GetDlgItem(IDC_EDIT_SRC_Y);
        pWnd->EnableWindow(FALSE);
        pWnd = GetDlgItem(IDC_EDIT_SRC_Z);
        pWnd->EnableWindow(FALSE);
    }
    else {
        CWnd* pWnd = GetDlgItem(IDC_EDIT_SRC_X);
        pWnd->EnableWindow(TRUE);
        pWnd = GetDlgItem(IDC_EDIT_SRC_Y);
        pWnd->EnableWindow(TRUE);
        pWnd = GetDlgItem(IDC_EDIT_SRC_Z);
        pWnd->EnableWindow(TRUE);
    }
    
	sprintf_s(buffer, fmt, srcX[sourceIdx]);
    SetDlgItemText(IDC_EDIT_SRC_X, buffer);
	sprintf_s(buffer, fmt, srcY[sourceIdx]);
    SetDlgItemText(IDC_EDIT_SRC_Y, buffer);
	sprintf_s(buffer, fmt, srcZ[sourceIdx]);
    SetDlgItemText(IDC_EDIT_SRC_Z, buffer);

    sprintf_s(buffer, fmt, headX);
    SetDlgItemText(IDC_EDIT_HEAD_X, buffer);
    sprintf_s(buffer, fmt, headY);
    SetDlgItemText(IDC_EDIT_HEAD_Y, buffer);
    sprintf_s(buffer, fmt, headZ);
    SetDlgItemText(IDC_EDIT_HEAD_Z, buffer);

    sprintf_s(buffer, fmt, roomWidth);
    SetDlgItemText(IDC_EDIT_ROOM_WIDTH, buffer);
    sprintf_s(buffer, fmt, roomHeight);
    SetDlgItemText(IDC_EDIT_ROOM_HEIGHT, buffer);
    sprintf_s(buffer, fmt, roomLength);
    SetDlgItemText(IDC_EDIT_ROOM_LENGTH, buffer);

    sprintf_s(buffer, fmt, roomDampLeft);
    SetDlgItemText(IDC_EDIT_ROOM_DAMP_LEFT, buffer);
    sprintf_s(buffer, fmt, roomDampRight);
    SetDlgItemText(IDC_EDIT_ROOM_DAMP_RIGHT, buffer);
    sprintf_s(buffer, fmt, roomDampFront);
    SetDlgItemText(IDC_EDIT_ROOM_DAMP_FRONT, buffer);
    sprintf_s(buffer, fmt, roomDampBack);
    SetDlgItemText(IDC_EDIT_ROOM_DAMP_BACK, buffer);
    sprintf_s(buffer, fmt, roomDampTop);
    SetDlgItemText(IDC_EDIT_ROOM_DAMP_TOP, buffer);
    sprintf_s(buffer, fmt, roomDampBottom);
    SetDlgItemText(IDC_EDIT_ROOM_DAMP_BOTTOM, buffer);

    sprintf_s(buffer, fmt, earSpacing);
    SetDlgItemText(IDC_EDIT_EARSPACING, buffer);

    CheckDlgButton(IDC_RADIO_CPU, !useGPU4Conv);
    CheckDlgButton(IDC_RADIO_GPU, useGPU4Conv );

	CheckDlgButton(IDC_RADIO_RG_CPU, !useGPU4Room);
    CheckDlgButton(IDC_RADIO_RG_GPU, useGPU4Room );

    CheckDlgButton(IDC_RADIO_FD, 1);

    sprintf_s(buffer, "%d", convolutionLength);
    SetDlgItemText(IDC_EDIT_CONV_LENGTH, buffer);
    sprintf_s(buffer, "%d", bufferSize);
    SetDlgItemText(IDC_EDIT_BUFFER_SIZE, buffer);
    //SetDlgItemText(IDC_EDITFILE, waveFileName);
    SetDlgItemText(IDC_EDITFILE, waveFileNames[sourceIdx]);

	pWnd = GetDlgItem(IDC_LIST_DEVICES_CONV);
	pWnd->EnableWindow(useGPU4Conv);
	for (int i = 0; i < MAX_DEVICES && deviceNames[i] != NULL; i++) {
		pWnd->SendMessage(LB_ADDSTRING, 0, (LPARAM)deviceNames[i]);
	}
	pWnd->SendMessage(LB_SETCURSEL, convDevIdx);


	pWnd = GetDlgItem(IDC_LIST_DEVICES_ROOM);
	pWnd->EnableWindow(useGPU4Room);
	for (int i = 0; i < MAX_DEVICES && deviceNames[i] != NULL; i++) {
		pWnd->SendMessage(LB_ADDSTRING, 0, (LPARAM)deviceNames[i]);
	}
	pWnd->SendMessage(LB_SETCURSEL, roomDevIdx);



	sprintf_s(buffer, "%d", nFiles);
	SetDlgItemText(IDC_EDIT_NFILES, buffer);
}

// CRoomAcousticsDlg message handlers

BOOL CRoomAcousticsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        CString strAboutMenu;
        if (strAboutMenu.LoadString(IDS_ABOUTBOX) && !strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    updateAllFields();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRoomAcousticsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialog::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRoomAcousticsDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRoomAcousticsDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


void CRoomAcousticsDlg::OnBnClickedButtonopen()
{

    CString fileName;
    char * p = fileName.GetBuffer(FILE_LIST_BUFFER_SIZE);
    CFileDialog dlgFile(TRUE);
    OPENFILENAME& ofn = dlgFile.GetOFN( );
    ofn.Flags |= OFN_ALLOWMULTISELECT;
    ofn.lpstrFile = p;
    ofn.nMaxFile = FILE_LIST_BUFFER_SIZE;

    dlgFile.DoModal();
    fileName.ReleaseBuffer();

    char * pBufEnd = p + FILE_LIST_BUFFER_SIZE - 2;
    char * start = p;
    while( ( p < pBufEnd ) && ( *p ) )
      p++;
    if( p > start )
    {
      _tprintf(_T("Path to folder where files were selected:  %s\r\n\r\n"), start );
      p++;
      if(*p == NULL){
          strncpy_s(waveFileNames[sourceIdx], MAX_PATH, start, MAX_PATH);
		  SetDlgItemText(IDC_EDITFILE, waveFileNames[sourceIdx]);
      }

      int fileCount = 1;
      while( ( p < pBufEnd ) && ( *p ) )
      {
        start = p;
        while( ( p < pBufEnd ) && ( *p ) )
          p++;
        if( p > start )
          _tprintf(_T("%2d. %s\r\n"), fileCount, start );
        p++;
        fileCount++;
      }
    }
}

void CRoomAcousticsDlg::OnEnKillfocusEditfile()
{
	GetDlgItemText(IDC_EDITFILE, waveFileNames[sourceIdx], MAX_PATH);
}

/*
float src1X,src1Y,src1Z;
float headX, headY, headZ;
float earSpacing;
float roomWidth, roomHeight, roomLength;
float roomDampLeft, roomDampRight, roomDampFront, roomDampBack, roomDampTop, roomDampBottom;
*/

void CRoomAcousticsDlg::updateRoomInfo()
{
    char buffer[160];
    RoomDefinition room;
    memset(&room, 0, sizeof(room));

    room.length = roomLength;
    room.width = roomWidth;
    room.height = roomHeight;
    room.mFront.damp = DBTODAMP(roomDampFront);
    room.mBack.damp = DBTODAMP(roomDampBack);
    room.mLeft.damp = DBTODAMP(roomDampLeft);
    room.mRight.damp = DBTODAMP(roomDampRight);
    room.mTop.damp = DBTODAMP(roomDampTop);
    room.mBottom.damp = DBTODAMP(roomDampBottom);

    //AmdTrueAudioVR *TrueAudioVR
    HMODULE TanVrDll;
    TanVrDll = LoadLibrary("TrueAudioVR.dll");
    if(NULL == TanVrDll)
    {
        ::AfxMessageBox(TEXT("NOT FOUND TrueAudioVR.dll"), MB_ICONSTOP);
        return;
    }
    typedef float  (WINAPI *estimateReverbTime)(RoomDefinition room, float finaldB, int *nReflections);
    estimateReverbTime estReverb = nullptr;

    estReverb = (estimateReverbTime)GetProcAddress(TanVrDll, "estimateReverbTime");

    int nR60, nR120;
    float t60 = estReverb(room, 60, &nR60);
    float t120 = estReverb(room, 120, &nR120);

    sprintf_s(buffer, 
        "Reverb time:\n\n T60 = %7.3f s,\n %d reflections.\n\n T120 = %7.3f s,\n %d reflections. ", t60,nR60,t120,nR120 );
    SetDlgItemText(IDC_ROOM_INFO, buffer);

}


void CRoomAcousticsDlg::OnEnChangeEditHeadX()
{
    char buffer[MAX_PATH];
    GetDlgItemText(IDC_EDIT_HEAD_X, buffer, MAX_PATH);
    sscanf_s(buffer, "%f", &headX);
}


void CRoomAcousticsDlg::OnEnChangeEditHeadY()
{
    char buffer[MAX_PATH];
    GetDlgItemText(IDC_EDIT_HEAD_Y, buffer, MAX_PATH);
    sscanf_s(buffer, "%f", &headY);
}


void CRoomAcousticsDlg::OnEnChangeEditHeadZ()
{
   char buffer[MAX_PATH];
    GetDlgItemText(IDC_EDIT_HEAD_Z, buffer, MAX_PATH);
    sscanf_s(buffer, "%f", &headZ);
}


void CRoomAcousticsDlg::OnEnChangeEditEarspacing()
{
   char buffer[MAX_PATH];
    GetDlgItemText(IDC_EDIT_EARSPACING, buffer, MAX_PATH);
    sscanf_s(buffer, "%f", &earSpacing);
}


void CRoomAcousticsDlg::OnEnChangeEditRoomWidth()
{
    char buffer[MAX_PATH];
    GetDlgItemText(IDC_EDIT_ROOM_WIDTH, buffer, MAX_PATH);
    sscanf_s(buffer, "%f", &roomWidth);
    updateRoomInfo();
}


void CRoomAcousticsDlg::OnEnChangeEditRoomHeight()
{
    char buffer[MAX_PATH];
    GetDlgItemText(IDC_EDIT_ROOM_HEIGHT, buffer, MAX_PATH);
    sscanf_s(buffer, "%f", &roomHeight);
    updateRoomInfo();
}


void CRoomAcousticsDlg::OnEnChangeEditRoomLength()
{
    char buffer[MAX_PATH];
    GetDlgItemText(IDC_EDIT_ROOM_LENGTH, buffer, MAX_PATH);
    sscanf_s(buffer, "%f", &roomLength);
    updateRoomInfo();
}


void CRoomAcousticsDlg::OnEnChangeEditRoomDampLeft()
{
     char buffer[MAX_PATH];
    GetDlgItemText(IDC_EDIT_ROOM_DAMP_LEFT, buffer, MAX_PATH);
    sscanf_s(buffer, "%f", &roomDampLeft);
    updateRoomInfo();
}


void CRoomAcousticsDlg::OnEnChangeEditRoomDampFront()
{
    char buffer[MAX_PATH];
    GetDlgItemText(IDC_EDIT_ROOM_DAMP_FRONT, buffer, MAX_PATH);
    sscanf_s(buffer, "%f", &roomDampFront);
    updateRoomInfo();
}


void CRoomAcousticsDlg::OnEnChangeEditRoomDampTop()
{
    char buffer[MAX_PATH];
    GetDlgItemText(IDC_EDIT_ROOM_DAMP_TOP, buffer, MAX_PATH);
    sscanf_s(buffer, "%f", &roomDampTop);
    updateRoomInfo();
}


void CRoomAcousticsDlg::OnEnChangeEditRoomDampRight()
{
    char buffer[MAX_PATH];
    GetDlgItemText(IDC_EDIT_ROOM_DAMP_RIGHT, buffer, MAX_PATH);
    sscanf_s(buffer, "%f", &roomDampRight);
    updateRoomInfo();
}


void CRoomAcousticsDlg::OnEnChangeEditRoomDampBack()
{
    char buffer[MAX_PATH];
    GetDlgItemText(IDC_EDIT_ROOM_DAMP_BACK, buffer, MAX_PATH);
    sscanf_s(buffer, "%f", &roomDampBack);
    updateRoomInfo();
}


void CRoomAcousticsDlg::OnEnChangeEditRoomDampBottom()
{
    char buffer[MAX_PATH];
    GetDlgItemText(IDC_EDIT_ROOM_DAMP_BOTTOM, buffer, MAX_PATH);
    sscanf_s(buffer, "%f", &roomDampBottom);
    updateRoomInfo();
}




void CRoomAcousticsDlg::OnEnChangeEditConvLength()
{
    char buffer[MAX_PATH];
    GetDlgItemText(IDC_EDIT_CONV_LENGTH, buffer, MAX_PATH);
    sscanf_s(buffer, "%d", &convolutionLength);
    sprintf_s(buffer,"%7.3f s @48kHz", convolutionLength/48000.0);
    SetDlgItemText(IDC_CONV_TIME, buffer);
}


void CRoomAcousticsDlg::OnEnChangeEditBufferSize()
{
    char buffer[MAX_PATH];
    GetDlgItemText(IDC_EDIT_BUFFER_SIZE, buffer, MAX_PATH);
    sscanf_s(buffer, "%d", &bufferSize);
    sprintf_s(buffer, "%7.3f s @48kHz", bufferSize / 48000.0);
    SetDlgItemText(IDC_BUF_TIME, buffer);
}


void CRoomAcousticsDlg::OnBnClickedRadioCpu()
{
    useGPU4Conv = 0;

	CWnd* pWnd = GetDlgItem(IDC_LIST_DEVICES_CONV);
	pWnd->EnableWindow(useGPU4Conv);
}

void CRoomAcousticsDlg::OnBnClickedRadioGpu()
{
    useGPU4Conv = 1;

	CWnd* pWnd = GetDlgItem(IDC_LIST_DEVICES_CONV);
	pWnd->EnableWindow(useGPU4Conv);
}


void CRoomAcousticsDlg::OnBnClickedRadioRgCpu()
{
	useGPU4Room = 0;

	CWnd* pWnd = GetDlgItem(IDC_LIST_DEVICES_ROOM);
	pWnd->EnableWindow(useGPU4Room);
}


void CRoomAcousticsDlg::OnBnClickedRadioRgGpu()
{
	useGPU4Room = 1;

	CWnd* pWnd = GetDlgItem(IDC_LIST_DEVICES_ROOM);
	pWnd->EnableWindow(useGPU4Room);
}


/*
    Very simple XML parser
    bool CRoomAcousticsDlg::findElement(char **start, char **end, char *name)
    finds one XML element between start and end
*/
bool CRoomAcousticsDlg::findElement(char **start, char **end, char *name)
{
    bool found = false;
    char *p = *start;
    while (p < *end){
        if (*p == '<')
        {
            if (strncmp(p+1, name, strlen(name)) == 0){
                *start = p++;
                int nestcount = 0;
                while (p < *end){
                    if (p[0] == '<')
                    {
                        if (p[1] == '/')
                        {
                            if (strncmp(p + 2, name, strlen(name)) == 0)
                            {
                                while (p < *end)
                                {
                                    if (*p++ == '>')
                                        break;
                                }
                                *end = p;
                                found = true;
                            }
                            ++p;
                        }
                        ++nestcount;
                    } else if (p[0] == '/' && p[1] == '>' && nestcount == 0){
                        p += 2;
                        *end = p;
                        found = true;
                    }
                    ++p;
                }
            }
            else {
                while (p < *end && *p != '>') {
                    ++p;
                }
            }
        }
        else {
            ++p;
        }
    }
    return found;
}

/*
  Very simple XML parser
  bool CRoomAcousticsDlg::parseElement(char *start, char *end, struct element *elem)
  Parses [recursively] one XML element.

*/

bool CRoomAcousticsDlg::parseElement(char *start, char *end, struct element *elem)
{ 
    bool ok = false;
    start += strlen(elem->name) + 1;

    // parse attributes
    for (int j = 0; j < elem->nAttribs; j++){
        char *pName = elem->attriblist[j].name;
        int len = (int) strlen(pName);
        char *p = start;
        while (p++ < end){
            if (strncmp(p, pName,len) == 0){
                p += len;
                while (p < end){
                    if (*p++ == '=')
                        break;
                }
                while (p < end){
                    if (*p++ == '\"')
                        break;
                }
                switch (elem->attriblist[j].fmt) {
                case 'f':
                    sscanf_s(p, "%f", (float *)elem->attriblist[j].value);
                    break;
                case 'i':
                    sscanf_s(p, "%d", (int *)elem->attriblist[j].value);
                    break;
                case 's':
                    {
                        char *sv = (char *)elem->attriblist[j].value;
                        while (p < end && *p != '\"'){
                            *sv++ = *p++;
                        }
                        *sv = '\0';
                    }
                    break;
                }
            }
        }
    }

    //parse included elements
    for (int i = 0; i < elem->nElements; i++){
        char *s, *e;
        s = start;
        e = end;
        if (findElement(&s, &e, elem->elemList[i].name)){
            ok = parseElement(s, e, &elem->elemList[i]);
        }
    }

    return ok;
}

/*
    void CRoomAcousticsDlg::LoadParameters(char * xmlFileName)
    Load parameters using Very Simple XML parser.

    Notes:  Does not support duplicate elements. Attribute names must not be subsets of other names in same list.
    Each tag has unique name tied to a single C++ variable.

*/
void CRoomAcousticsDlg::LoadParameters(char * xmlFileName)
{
    useGPU4Conv = 0;
    useGPU4Room = 0;

	struct attribute src1PosAttribs[3] = { { "X", &srcX[0], 'f' }, { "Y", &srcY[0], 'f' }, { "Z", &srcZ[0], 'f' } };
	struct attribute src2PosAttribs[3] = { { "X", &srcX[1], 'f' }, { "Y", &srcY[1], 'f' }, { "Z", &srcZ[1], 'f' } };
	struct attribute src3PosAttribs[3] = { { "X", &srcX[2], 'f' }, { "Y", &srcY[2], 'f' }, { "Z", &srcZ[2], 'f' } };
	struct attribute src4PosAttribs[3] = { { "X", &srcX[3], 'f' }, { "Y", &srcY[3], 'f' }, { "Z", &srcZ[3], 'f' } };
	struct attribute src5PosAttribs[3] = { { "X", &srcX[4], 'f' }, { "Y", &srcY[4], 'f' }, { "Z", &srcZ[4], 'f' } };
	struct attribute src6PosAttribs[3] = { { "X", &srcX[5], 'f' }, { "Y", &srcY[5], 'f' }, { "Z", &srcZ[5], 'f' } };
	struct attribute src7PosAttribs[3] = { { "X", &srcX[6], 'f' }, { "Y", &srcY[6], 'f' }, { "Z", &srcZ[6], 'f' } };
	struct attribute src8PosAttribs[3] = { { "X", &srcX[7], 'f' }, { "Y", &srcY[7], 'f' }, { "Z", &srcZ[7], 'f' } };
	struct attribute src9PosAttribs[3] = { { "X", &srcX[8], 'f' }, { "Y", &srcY[8], 'f' }, { "Z", &srcZ[8], 'f' } };
	struct attribute src10PosAttribs[3] = { { "X", &srcX[9], 'f' }, { "Y", &srcY[9], 'f' }, { "Z", &srcZ[9], 'f' } };
    struct attribute src11PosAttribs[3] = { { "X", &srcX[10], 'f' }, { "Y", &srcY[10], 'f' }, { "Z", &srcZ[10], 'f' } };
    struct attribute src12PosAttribs[3] = { { "X", &srcX[11], 'f' }, { "Y", &srcY[11], 'f' }, { "Z", &srcZ[11], 'f' } };
    struct attribute src13PosAttribs[3] = { { "X", &srcX[12], 'f' }, { "Y", &srcY[12], 'f' }, { "Z", &srcZ[12], 'f' } };
    struct attribute src14PosAttribs[3] = { { "X", &srcX[13], 'f' }, { "Y", &srcY[13], 'f' }, { "Z", &srcZ[13], 'f' } };
    struct attribute src15PosAttribs[3] = { { "X", &srcX[14], 'f' }, { "Y", &srcY[14], 'f' }, { "Z", &srcZ[14], 'f' } };
    struct attribute src16PosAttribs[3] = { { "X", &srcX[15], 'f' }, { "Y", &srcY[15], 'f' }, { "Z", &srcZ[15], 'f' } };
    struct attribute src17PosAttribs[3] = { { "X", &srcX[16], 'f' }, { "Y", &srcY[16], 'f' }, { "Z", &srcZ[16], 'f' } };
    struct attribute src18PosAttribs[3] = { { "X", &srcX[17], 'f' }, { "Y", &srcY[17], 'f' }, { "Z", &srcZ[17], 'f' } };
    struct attribute src19PosAttribs[3] = { { "X", &srcX[18], 'f' }, { "Y", &srcY[18], 'f' }, { "Z", &srcZ[18], 'f' } };
    struct attribute src20PosAttribs[3] = { { "X", &srcX[19], 'f' }, { "Y", &srcY[19], 'f' }, { "Z", &srcZ[19], 'f' } };
    struct attribute src21PosAttribs[3] = { { "X", &srcX[20], 'f' }, { "Y", &srcY[20], 'f' }, { "Z", &srcZ[20], 'f' } };
    struct attribute src22PosAttribs[3] = { { "X", &srcX[21], 'f' }, { "Y", &srcY[21], 'f' }, { "Z", &srcZ[21], 'f' } };
    struct attribute src23PosAttribs[3] = { { "X", &srcX[22], 'f' }, { "Y", &srcY[22], 'f' }, { "Z", &srcZ[22], 'f' } };
    struct attribute src24PosAttribs[3] = { { "X", &srcX[23], 'f' }, { "Y", &srcY[23], 'f' }, { "Z", &srcZ[23], 'f' } };
    struct attribute src25PosAttribs[3] = { { "X", &srcX[24], 'f' }, { "Y", &srcY[24], 'f' }, { "Z", &srcZ[24], 'f' } };
    struct attribute src26PosAttribs[3] = { { "X", &srcX[25], 'f' }, { "Y", &srcY[25], 'f' }, { "Z", &srcZ[25], 'f' } };
    struct attribute src27PosAttribs[3] = { { "X", &srcX[26], 'f' }, { "Y", &srcY[26], 'f' }, { "Z", &srcZ[26], 'f' } };
    struct attribute src28PosAttribs[3] = { { "X", &srcX[27], 'f' }, { "Y", &srcY[27], 'f' }, { "Z", &srcZ[27], 'f' } };
    struct attribute src29PosAttribs[3] = { { "X", &srcX[28], 'f' }, { "Y", &srcY[28], 'f' }, { "Z", &srcZ[28], 'f' } };
    struct attribute src30PosAttribs[3] = { { "X", &srcX[29], 'f' }, { "Y", &srcY[29], 'f' }, { "Z", &srcZ[29], 'f' } };
    struct attribute src31PosAttribs[3] = { { "X", &srcX[30], 'f' }, { "Y", &srcY[30], 'f' }, { "Z", &srcZ[30], 'f' } };
    struct attribute src32PosAttribs[3] = { { "X", &srcX[31], 'f' }, { "Y", &srcY[31], 'f' }, { "Z", &srcZ[31], 'f' } };
    struct attribute src33PosAttribs[3] = { { "X", &srcX[32], 'f' }, { "Y", &srcY[32], 'f' }, { "Z", &srcZ[32], 'f' } };
    struct attribute src34PosAttribs[3] = { { "X", &srcX[33], 'f' }, { "Y", &srcY[33], 'f' }, { "Z", &srcZ[33], 'f' } };
    struct attribute src35PosAttribs[3] = { { "X", &srcX[34], 'f' }, { "Y", &srcY[34], 'f' }, { "Z", &srcZ[34], 'f' } };
    struct attribute src36PosAttribs[3] = { { "X", &srcX[35], 'f' }, { "Y", &srcY[35], 'f' }, { "Z", &srcZ[35], 'f' } };
    struct attribute src37PosAttribs[3] = { { "X", &srcX[36], 'f' }, { "Y", &srcY[36], 'f' }, { "Z", &srcZ[36], 'f' } };
    struct attribute src38PosAttribs[3] = { { "X", &srcX[37], 'f' }, { "Y", &srcY[37], 'f' }, { "Z", &srcZ[37], 'f' } };
    struct attribute src39PosAttribs[3] = { { "X", &srcX[38], 'f' }, { "Y", &srcY[38], 'f' }, { "Z", &srcZ[38], 'f' } };
    struct attribute src40PosAttribs[3] = { { "X", &srcX[39], 'f' }, { "Y", &srcY[39], 'f' }, { "Z", &srcZ[39], 'f' } };


    struct attribute src1MicAttribs[3] = { { "enableMic", &src1EnableMic, 'i' }, { "trackHeadPos", &src1TrackHeadPos, 'i' }, { "muteDirectPath", &src1MuteDirectPath, 'i' } };
    struct attribute headPosAttribs[6] = { { "X", &headX, 'f' }, { "Y", &headY, 'f' }, { "Z", &headZ, 'f' },
                                           { "yaw", &yaw, 'f' }, { "pitch", &pitch, 'f' }, { "roll", &roll, 'f' } };
    struct attribute roomDimAttribs[3] = { { "width", &roomWidth, 'f' }, { "height", &roomHeight, 'f' }, { "length", &roomLength, 'f' } };
    struct attribute roomDampAttribs[6] =
    { { "left", &roomDampLeft, 'f' }, { "right", &roomDampRight, 'f' },
    { "front", &roomDampFront, 'f' }, { "back", &roomDampBack, 'f' },
    { "top", &roomDampTop, 'f' }, { "bottom", &roomDampBottom, 'f' } };

    struct attribute roomRenderAttribs[2] = { { "nSources", &nFiles, 'i' }, { "withGPU", &useGPU4Room, 'i' } };

    struct attribute earAttribs[1] = { { "S", &earSpacing, 'f' } };
    struct attribute stream1Attribs[1] = { { "file", &waveFileNames[0][0], 's' } };
	struct attribute stream2Attribs[2] = { { "on", &srcEnable[1], 'i' }, { "file", &waveFileNames[1][0], 's' } };
	struct attribute stream3Attribs[2] = { { "on", &srcEnable[2], 'i' }, { "file", &waveFileNames[2][0], 's' } };
	struct attribute stream4Attribs[2] = { { "on", &srcEnable[3], 'i' }, { "file", &waveFileNames[3][0], 's' } };
	struct attribute stream5Attribs[2] = { { "on", &srcEnable[4], 'i' }, { "file", &waveFileNames[4][0], 's' } };
	struct attribute stream6Attribs[2] = { { "on", &srcEnable[5], 'i' }, { "file", &waveFileNames[5][0], 's' } };
	struct attribute stream7Attribs[2] = { { "on", &srcEnable[6], 'i' }, { "file", &waveFileNames[6][0], 's' } };
	struct attribute stream8Attribs[2] = { { "on", &srcEnable[7], 'i' }, { "file", &waveFileNames[7][0], 's' } };
	struct attribute stream9Attribs[2] = { { "on", &srcEnable[8], 'i' }, { "file", &waveFileNames[8][0], 's' } };
	struct attribute stream10Attribs[2] = { { "on", &srcEnable[9], 'i' }, { "file", &waveFileNames[9][0], 's' } };
    struct attribute stream11Attribs[2] = { { "on", &srcEnable[10], 'i' }, { "file", &waveFileNames[10][0], 's' } };
    struct attribute stream12Attribs[2] = { { "on", &srcEnable[11], 'i' }, { "file", &waveFileNames[11][0], 's' } };
    struct attribute stream13Attribs[2] = { { "on", &srcEnable[12], 'i' }, { "file", &waveFileNames[12][0], 's' } };
    struct attribute stream14Attribs[2] = { { "on", &srcEnable[13], 'i' }, { "file", &waveFileNames[13][0], 's' } };
    struct attribute stream15Attribs[2] = { { "on", &srcEnable[14], 'i' }, { "file", &waveFileNames[14][0], 's' } };
    struct attribute stream16Attribs[2] = { { "on", &srcEnable[15], 'i' }, { "file", &waveFileNames[15][0], 's' } };
    struct attribute stream17Attribs[2] = { { "on", &srcEnable[16], 'i' }, { "file", &waveFileNames[16][0], 's' } };
    struct attribute stream18Attribs[2] = { { "on", &srcEnable[17], 'i' }, { "file", &waveFileNames[17][0], 's' } };
    struct attribute stream19Attribs[2] = { { "on", &srcEnable[18], 'i' }, { "file", &waveFileNames[18][0], 's' } };
    struct attribute stream20Attribs[2] = { { "on", &srcEnable[19], 'i' }, { "file", &waveFileNames[19][0], 's' } };
    struct attribute stream21Attribs[2] = { { "on", &srcEnable[20], 'i' }, { "file", &waveFileNames[20][0], 's' } };
    struct attribute stream22Attribs[2] = { { "on", &srcEnable[21], 'i' }, { "file", &waveFileNames[21][0], 's' } };
    struct attribute stream23Attribs[2] = { { "on", &srcEnable[22], 'i' }, { "file", &waveFileNames[22][0], 's' } };
    struct attribute stream24Attribs[2] = { { "on", &srcEnable[23], 'i' }, { "file", &waveFileNames[23][0], 's' } };
    struct attribute stream25Attribs[2] = { { "on", &srcEnable[24], 'i' }, { "file", &waveFileNames[24][0], 's' } };
    struct attribute stream26Attribs[2] = { { "on", &srcEnable[25], 'i' }, { "file", &waveFileNames[25][0], 's' } };
    struct attribute stream27Attribs[2] = { { "on", &srcEnable[26], 'i' }, { "file", &waveFileNames[26][0], 's' } };
    struct attribute stream28Attribs[2] = { { "on", &srcEnable[27], 'i' }, { "file", &waveFileNames[27][0], 's' } };
    struct attribute stream29Attribs[2] = { { "on", &srcEnable[28], 'i' }, { "file", &waveFileNames[28][0], 's' } };
    struct attribute stream30Attribs[2] = { { "on", &srcEnable[29], 'i' }, { "file", &waveFileNames[29][0], 's' } };
    struct attribute stream31Attribs[2] = { { "on", &srcEnable[30], 'i' }, { "file", &waveFileNames[30][0], 's' } };
    struct attribute stream32Attribs[2] = { { "on", &srcEnable[31], 'i' }, { "file", &waveFileNames[31][0], 's' } };
    struct attribute stream33Attribs[2] = { { "on", &srcEnable[32], 'i' }, { "file", &waveFileNames[32][0], 's' } };
    struct attribute stream34Attribs[2] = { { "on", &srcEnable[33], 'i' }, { "file", &waveFileNames[33][0], 's' } };
    struct attribute stream35Attribs[2] = { { "on", &srcEnable[34], 'i' }, { "file", &waveFileNames[34][0], 's' } };
    struct attribute stream36Attribs[2] = { { "on", &srcEnable[35], 'i' }, { "file", &waveFileNames[35][0], 's' } };
    struct attribute stream37Attribs[2] = { { "on", &srcEnable[36], 'i' }, { "file", &waveFileNames[36][0], 's' } };
    struct attribute stream38Attribs[2] = { { "on", &srcEnable[37], 'i' }, { "file", &waveFileNames[37][0], 's' } };
    struct attribute stream39Attribs[2] = { { "on", &srcEnable[38], 'i' }, { "file", &waveFileNames[38][0], 's' } };
    struct attribute stream40Attribs[2] = { { "on", &srcEnable[39], 'i' }, { "file", &waveFileNames[39][0], 's' } };


	struct attribute convCfgAttribs[3] = {
        { "length", &convolutionLength, 'i' },
        { "buffersize", &bufferSize, 'i' },
        { "useGPU", &useGPU4Conv, 'i' }
         };


    struct element src1[3] = {
        { "streamS1", 1, stream1Attribs, 0, NULL },
        { "microphone", 3, src1MicAttribs, 0, NULL },
        { "positionS1", 3, src1PosAttribs, 0, NULL }
    };
    struct element src2[2] = {
        { "streamS2", 2, stream2Attribs, 0, NULL },
        { "positionS2", 3, src2PosAttribs, 0, NULL }
    };

	struct element src3[2] = {
		{ "streamS3", 2, stream3Attribs, 0, NULL },
		{ "positionS3", 3, src3PosAttribs, 0, NULL }
	};
	struct element src4[2] = {
		{ "streamS4", 2, stream4Attribs, 0, NULL },
		{ "positionS4", 3, src4PosAttribs, 0, NULL }
	};
	struct element src5[2] = {
		{ "streamS5", 2, stream5Attribs, 0, NULL },
		{ "positionS5", 3, src5PosAttribs, 0, NULL }
	};
	struct element src6[2] = {
		{ "streamS6", 2, stream6Attribs, 0, NULL },
		{ "positionS6", 3, src6PosAttribs, 0, NULL }
	};
	struct element src7[2] = {
		{ "streamS7", 2, stream7Attribs, 0, NULL },
		{ "positionS7", 3, src7PosAttribs, 0, NULL }
	};
	struct element src8[2] = {
		{ "streamS8", 2, stream8Attribs, 0, NULL },
		{ "positionS8", 3, src8PosAttribs, 0, NULL }
	};
	struct element src9[2] = {
		{ "streamS9", 2, stream9Attribs, 0, NULL },
		{ "positionS9", 3, src9PosAttribs, 0, NULL }
	};
	struct element src10[2] = {
		{ "streamS10", 2, stream10Attribs, 0, NULL },
		{ "positionS10", 3, src10PosAttribs, 0, NULL }
	};
    struct element src11[2] = {
        { "streamS11", 2, stream11Attribs, 0, NULL },
        { "positionS11", 3, src11PosAttribs, 0, NULL }
    };
    struct element src12[2] = {
        { "streamS12", 2, stream12Attribs, 0, NULL },
        { "positionS12", 3, src12PosAttribs, 0, NULL }
    };
    struct element src13[2] = {
        { "streamS13", 2, stream13Attribs, 0, NULL },
        { "positionS13", 3, src13PosAttribs, 0, NULL }
    };
    struct element src14[2] = {
        { "streamS14", 2, stream14Attribs, 0, NULL },
        { "positionS14", 3, src14PosAttribs, 0, NULL }
    };
    struct element src15[2] = {
        { "streamS15", 2, stream15Attribs, 0, NULL },
        { "positionS15", 3, src15PosAttribs, 0, NULL }
    };
    struct element src16[2] = {
        { "streamS16", 2, stream16Attribs, 0, NULL },
        { "positionS16", 3, src16PosAttribs, 0, NULL }
    };
    struct element src17[2] = {
        { "streamS17", 2, stream17Attribs, 0, NULL },
        { "positionS17", 3, src17PosAttribs, 0, NULL }
    };
    struct element src18[2] = {
        { "streamS18", 2, stream18Attribs, 0, NULL },
        { "positionS18", 3, src18PosAttribs, 0, NULL }
    };
    struct element src19[2] = {
        { "streamS19", 2, stream19Attribs, 0, NULL },
        { "positionS19", 3, src19PosAttribs, 0, NULL }
    };
    struct element src20[2] = {
        { "streamS20", 2, stream20Attribs, 0, NULL },
        { "positionS20", 3, src20PosAttribs, 0, NULL }
    };
    struct element src21[2] = {
        { "streamS21", 2, stream21Attribs, 0, NULL },
        { "positionS21", 3, src21PosAttribs, 0, NULL }
    };
    struct element src22[2] = {
        { "streamS22", 2, stream22Attribs, 0, NULL },
        { "positionS22", 3, src22PosAttribs, 0, NULL }
    };
    struct element src23[2] = {
        { "streamS23", 2, stream23Attribs, 0, NULL },
        { "positionS23", 3, src23PosAttribs, 0, NULL }
    };
    struct element src24[2] = {
        { "streamS24", 2, stream24Attribs, 0, NULL },
        { "positionS24", 3, src24PosAttribs, 0, NULL }
    };
    struct element src25[2] = {
        { "streamS25", 2, stream25Attribs, 0, NULL },
        { "positionS25", 3, src25PosAttribs, 0, NULL }
    };
    struct element src26[2] = {
        { "streamS26", 2, stream26Attribs, 0, NULL },
        { "positionS26", 3, src26PosAttribs, 0, NULL }
    };
    struct element src27[2] = {
        { "streamS27", 2, stream27Attribs, 0, NULL },
        { "positionS27", 3, src27PosAttribs, 0, NULL }
    };
    struct element src28[2] = {
        { "streamS28", 2, stream28Attribs, 0, NULL },
        { "positionS28", 3, src28PosAttribs, 0, NULL }
    };
    struct element src29[2] = {
        {   "streamS29", 2, stream29Attribs, 0, NULL },
        { "positionS29", 3,    src29PosAttribs, 0, NULL }
    };
    struct element src30[2] = {
        { "streamS30", 2, stream30Attribs, 0, NULL },
        { "positionS30", 3, src30PosAttribs, 0, NULL }
    };
    struct element src31[2] = {
        { "streamS31", 2, stream31Attribs, 0, NULL },
        { "positionS31", 3, src31PosAttribs, 0, NULL }
    };
    struct element src32[2] = {
        { "streamS32", 2, stream32Attribs, 0, NULL },
        { "positionS32", 3, src32PosAttribs, 0, NULL }
    };
    struct element src33[2] = {
        { "streamS33", 2, stream33Attribs, 0, NULL },
        { "positionS33", 3, src33PosAttribs, 0, NULL }
    };
    struct element src34[2] = {
        { "streamS34", 2, stream34Attribs, 0, NULL },
        { "positionS34", 3, src34PosAttribs, 0, NULL }
    };
    struct element src35[2] = {
        { "streamS35", 2, stream35Attribs, 0, NULL },
        { "positionS35", 3, src35PosAttribs, 0, NULL }
    };
    struct element src36[2] = {
        { "streamS36", 2, stream36Attribs, 0, NULL },
        { "positionS36", 3, src36PosAttribs, 0, NULL }
    };
    struct element src37[2] = {
        { "streamS37", 2, stream37Attribs, 0, NULL },
        { "positionS37", 3, src37PosAttribs, 0, NULL }
    };
    struct element src38[2] = {
        { "streamS38", 2, stream38Attribs, 0, NULL },
        { "positionS38", 3, src38PosAttribs, 0, NULL }
    };
    struct element src39[2] = {
        { "streamS39", 2, stream39Attribs, 0, NULL },
        { "positionS39", 3, src39PosAttribs, 0, NULL }
    };
    struct element src40[2] = {
        { "streamS40", 2, stream40Attribs, 0, NULL },
        { "positionS40", 3, src40PosAttribs, 0, NULL }
    };

    struct element head[2] = {
        { "positionL1", 6, headPosAttribs, 0, NULL },
        { "earSpacing", 1, earAttribs, 0, NULL },
    };

    struct element roomElems[3] = {
        { "dimensions", 3, roomDimAttribs, NULL },
        { "damping", 6, roomDampAttribs, NULL },
		{ "rendering", 2, roomRenderAttribs, NULL }
    };

    struct element convElems[1] = {
        { "configuration", 3, convCfgAttribs, 0, NULL }
    };

    struct element RAelementList[43] =
    {
        { "Source1", 0, NULL, sizeof(src1) / sizeof(element), src1 },
        { "Source2", 0, NULL, sizeof(src2) / sizeof(element), src2 },
		{ "Source3", 0, NULL, sizeof(src3) / sizeof(element), src3 },
		{ "Source4", 0, NULL, sizeof(src4) / sizeof(element), src4 },
		{ "Source5", 0, NULL, sizeof(src5) / sizeof(element), src5 },
		{ "Source6", 0, NULL, sizeof(src6) / sizeof(element), src6 },
		{ "Source7", 0, NULL, sizeof(src7) / sizeof(element), src7 },
		{ "Source8", 0, NULL, sizeof(src8) / sizeof(element), src8 },
		{ "Source9", 0, NULL, sizeof(src9) / sizeof(element), src9 },
		{ "Source10", 0, NULL, sizeof(src10) / sizeof(element), src10 },
        { "Source11", 0, NULL, sizeof(src11) / sizeof(element), src11 },
        { "Source12", 0, NULL, sizeof(src12) / sizeof(element), src12 },
        { "Source13", 0, NULL, sizeof(src13) / sizeof(element), src13 },
        { "Source14", 0, NULL, sizeof(src14) / sizeof(element), src14 },
        { "Source15", 0, NULL, sizeof(src15) / sizeof(element), src15 },
        { "Source16", 0, NULL, sizeof(src16) / sizeof(element), src16 },
        { "Source17", 0, NULL, sizeof(src17) / sizeof(element), src17 },
        { "Source18", 0, NULL, sizeof(src18) / sizeof(element), src18 },
        { "Source19", 0, NULL, sizeof(src19) / sizeof(element), src19 },
        { "Source20", 0, NULL, sizeof(src20) / sizeof(element), src20 },
        { "Source21", 0, NULL, sizeof(src21) / sizeof(element), src21 },
        { "Source22", 0, NULL, sizeof(src22) / sizeof(element), src22 },
        { "Source23", 0, NULL, sizeof(src23) / sizeof(element), src23 },
        { "Source24", 0, NULL, sizeof(src24) / sizeof(element), src24 },
        { "Source25", 0, NULL, sizeof(src25) / sizeof(element), src25 },
        { "Source26", 0, NULL, sizeof(src26) / sizeof(element), src26 },
        { "Source27", 0, NULL, sizeof(src27) / sizeof(element), src27 },
        { "Source28", 0, NULL, sizeof(src28) / sizeof(element), src28 },
        { "Source29", 0, NULL, sizeof(src29) / sizeof(element), src29 },
        { "Source30", 0, NULL, sizeof(src30) / sizeof(element), src30 },
        { "Source31", 0, NULL, sizeof(src31) / sizeof(element), src31 },
        { "Source32", 0, NULL, sizeof(src32) / sizeof(element), src32 },
        { "Source33", 0, NULL, sizeof(src33) / sizeof(element), src33 },
        { "Source34", 0, NULL, sizeof(src34) / sizeof(element), src34 },
        { "Source35", 0, NULL, sizeof(src35) / sizeof(element), src35 },
        { "Source36", 0, NULL, sizeof(src36) / sizeof(element), src36 },
        { "Source37", 0, NULL, sizeof(src37) / sizeof(element), src37 },
        { "Source38", 0, NULL, sizeof(src38) / sizeof(element), src38 },
        { "Source39", 0, NULL, sizeof(src39) / sizeof(element), src39 },
        { "Source40", 0, NULL, sizeof(src40) / sizeof(element), src40 },

		{ "Listener", 0, NULL, sizeof(head) / sizeof(element), head },
        { "Room", 0, NULL,        sizeof(roomElems)/sizeof(element), roomElems },
        { "Convolution", 0, NULL, sizeof(convElems)/sizeof(element), convElems }
    };

    struct element RoomAcoustics = { "RoomAcoustics", 0, NULL, sizeof(RAelementList) / sizeof(element), RAelementList };

    // read xml file
    FILE *fpLoadFile = NULL;

    fopen_s(&fpLoadFile, xmlFileName, "r+");

    if (fpLoadFile == NULL){
        return;
    }

    fseek(fpLoadFile, 0, SEEK_END);
    int fLen = ftell(fpLoadFile);
    fseek(fpLoadFile, 0, SEEK_SET);

    char *xmlBuf = (char *)calloc(fLen, 1);
    if (!xmlBuf) return;

    fread(xmlBuf, 1, fLen, fpLoadFile);

    char *start, *end;
    start = xmlBuf;
    end = start + fLen;
    parseElement(start, end, &RoomAcoustics);

	fclose(fpLoadFile);

}


void CRoomAcousticsDlg::OnBnClickedLoad()
{

    // read xml file
    FILE *fpLoadFile = NULL;
    CString fileName = "default";
    char * p = fileName.GetBuffer(FILE_LIST_BUFFER_SIZE);
    CFileDialog dlgFile(TRUE, "xml", fileName, 0, "XML Files (*.xml)||", NULL, 0, 0);
    OPENFILENAME& ofn = dlgFile.GetOFN();
    ofn.Flags |= OFN_ALLOWMULTISELECT;
    ofn.lpstrFile = p;
    ofn.nMaxFile = FILE_LIST_BUFFER_SIZE;

    dlgFile.DoModal();
    fileName.ReleaseBuffer();

    LoadParameters(p);
    updateAllFields();

}


void CRoomAcousticsDlg::OnBnClickedSave()
{
    time_t dt = time(NULL);
    struct tm *lt = localtime(&dt);
    FILE *fpSaveFile = NULL;

    CString fileName = "default";
    char * p = fileName.GetBuffer(FILE_LIST_BUFFER_SIZE);
    CFileDialog dlgFile(FALSE, "xml", fileName, 0, "XML Files (*xml)||", NULL, 0, 0);
    OPENFILENAME& ofn = dlgFile.GetOFN();
    ofn.Flags |= OFN_ALLOWMULTISELECT;
    ofn.lpstrFile = p;
    ofn.nMaxFile = FILE_LIST_BUFFER_SIZE;

    dlgFile.DoModal();
    fileName.ReleaseBuffer();
    
    fopen_s(&fpSaveFile, p, "w+");

    if (fpSaveFile == NULL){
        return;
    }

    fputs("<?xml version='1.0' encoding='UTF-8'?>\n",fpSaveFile);
    fprintf(fpSaveFile,
        "<!-- This document was created by AMD RoomAcousticsDemo v1.0.2 on %4d/%02d/%02d %02d:%02d:%02d -->\n",
        2000 + (lt->tm_year % 100), 1 + lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
    fputs("<RoomAcoustics>\n", fpSaveFile);
    fputs("<!-- All dimensions in meters, damping in decibels -->\n", fpSaveFile);

		fputs(" <Source1>\n", fpSaveFile);
		fprintf(fpSaveFile, " <streamS1  on=\"%d\" file=\"%s\" />\n", srcEnable[0], waveFileNames[0]);
		fprintf(fpSaveFile, "  <microphone enableMic=\"%d\" trackHeadPos=\"%d\" muteDirectPath=\"%d\" />\n",
			src1EnableMic, src1TrackHeadPos, src1MuteDirectPath);
		fprintf(fpSaveFile, "  <positionS1 X=\"%f\" Y=\"%f\" Z=\"%f\"  />\n", srcX[0], srcY[0], srcZ[0]);
		fputs(" </Source1>\n", fpSaveFile);

		for (int i = 1; i < nFiles; i++) {
			fprintf(fpSaveFile," <Source%d>\n",i+1 );
			fprintf(fpSaveFile, " <streamS%d  on=\"%d\" file=\"%s\" />\n", i + 1, srcEnable[i], waveFileNames[i]);
			fprintf(fpSaveFile, "  <positionS%d X=\"%f\" Y=\"%f\" Z=\"%f\"  />\n", i+1, srcX[i], srcY[i], srcZ[i]);
			fprintf(fpSaveFile, " </Source%d>\n", i + 1);
		}

        fputs(" <Listener>\n", fpSaveFile);
            fprintf(fpSaveFile, "  <positionL1 X=\"%f\" Y=\"%f\" Z=\"%f\" yaw=\"%f\" pitch=\"%f\" roll=\"%f\" />\n", 
                headX, headY, headZ, yaw, pitch, roll);
            fprintf(fpSaveFile, "  <earSpacing S=\"%f\"/>\n", earSpacing);

        fputs(" </Listener>\n", fpSaveFile);
        fputs(" <Room>\n", fpSaveFile);
        fprintf(fpSaveFile, "  <dimensions width=\"%f\" height=\"%f\" length=\"%f\" />\n", roomWidth, roomHeight, roomLength);
        fprintf(fpSaveFile, "  <damping left=\"%f\" right=\"%f\" front=\"%f\" back=\"%f\" top=\"%f\" bottom=\"%f\"/>\n", 
            roomDampLeft, roomDampRight, roomDampFront, roomDampBack, roomDampTop, roomDampBottom);
		fprintf(fpSaveFile, " <rendering nSources=\"%d\" withGPU=\"%d\" />\n", nFiles, useGPU4Room);
		fputs(" </Room>\n", fpSaveFile);
        fputs("<Convolution>\n", fpSaveFile);
           fprintf(fpSaveFile, " <configuration length=\"%d\" buffersize =\"%d\" useGPU=\"%d\" />\n",
               convolutionLength, bufferSize, useGPU4Conv);
        fputs("</Convolution>\n", fpSaveFile);
    fputs("</RoomAcoustics>\n", fpSaveFile);
    fclose(fpSaveFile);
}


void CRoomAcousticsDlg::OnBnClickedExport()
{
    CString fileName = "roomresponse";
    char * pfName = fileName.GetBuffer(FILE_LIST_BUFFER_SIZE);
    CFileDialog dlgFile(FALSE, "wav", fileName, 0, "XML Files (*wav)||", NULL, 0, 0);
    OPENFILENAME& ofn = dlgFile.GetOFN();
    ofn.Flags |= OFN_ALLOWMULTISELECT;
    ofn.lpstrFile = pfName;
    ofn.nMaxFile = FILE_LIST_BUFFER_SIZE;

    dlgFile.DoModal();
    fileName.ReleaseBuffer();

    RoomDefinition room;
    memset(&room, 0, sizeof(room));
    room.length = roomLength;
    room.width = roomWidth;
    room.height = roomHeight;
    room.mFront.damp = DBTODAMP(roomDampFront);
    room.mBack.damp = DBTODAMP(roomDampBack);
    room.mLeft.damp = DBTODAMP(roomDampLeft);
    room.mRight.damp = DBTODAMP(roomDampRight);
    room.mTop.damp = DBTODAMP(roomDampTop);
    room.mBottom.damp = DBTODAMP(roomDampBottom);
     
    MonoSource src;
    src.speakerX = srcX[0];
    src.speakerY = srcY[0];
    src.speakerZ = srcZ[0];
    StereoListener ears;
    ears.earSpacing = earSpacing;
    ears.headX = this->headX;
    ears.headY = this->headY;
    ears.headZ = this->headZ;
    ears.pitch = this->pitch;
    ears.roll = this->roll;
    ears.yaw = this->yaw;
    m_pTAVR->generateSimpleHeadRelatedTransform(&ears.hrtf, ears.earSpacing);

    float *leftResponse = new float[convolutionLength];
    float *rightResponse = new float[convolutionLength];
    short *sSamples = new short[2 * convolutionLength];
    memset(leftResponse, 0, convolutionLength*sizeof(float));
    memset(rightResponse, 0, convolutionLength*sizeof(float));
    memset(sSamples, 0, 2 * convolutionLength*sizeof(short));

    m_pTAVR->generateRoomResponse(room, src, ears, 48000, convolutionLength, leftResponse, rightResponse);

    (void)m_spConverter->Convert(leftResponse, 1, convolutionLength, sSamples, 2, 1.f);
    //m_pTA->NormFloatsToShorts(leftResponse, sSamples, convolutionLength, 2, 1.0);

    (void)m_spConverter->Convert(rightResponse, 1, convolutionLength, sSamples + 1, 2, 1.f);
    //m_pTA->NormFloatsToShorts(rightResponse, sSamples + 1, convolutionLength, 2, 1.0);

    WriteWaveFileS(pfName, 48000, 2, 16, convolutionLength, sSamples);
    delete[] leftResponse;
    delete[] rightResponse;
    delete[] sSamples;
}


void CRoomAcousticsDlg::OnBnClickedCheckEnSrc()
{
    srcEnable[sourceIdx] = IsDlgButtonChecked(IDC_CHECK_EN_SRC);
}


void CRoomAcousticsDlg::OnEnChangeEditSrcX()
{
    char buffer[MAX_PATH];
	int srcIdx;
	CWnd *pWnd;
	pWnd = GetDlgItem(IDC_LIST_SOURCES);
	srcIdx = (int) pWnd->SendMessage(LB_GETCURSEL, 0, (LPARAM)0);
	GetDlgItemText(IDC_EDIT_SRC_X, buffer, MAX_PATH);
	sscanf_s(buffer, "%f", &srcX[srcIdx]);

}


void CRoomAcousticsDlg::OnEnChangeEditSrcY()
{
    char buffer[MAX_PATH];
	int srcIdx;
	CWnd *pWnd;
	pWnd = GetDlgItem(IDC_LIST_SOURCES);
	srcIdx = (int) pWnd->SendMessage(LB_GETCURSEL, 0, (LPARAM)0);
	GetDlgItemText(IDC_EDIT_SRC_Y, buffer, MAX_PATH);
	sscanf_s(buffer, "%f", &srcY[srcIdx]);
}


void CRoomAcousticsDlg::OnEnChangeEditSrcZ()
{
    char buffer[MAX_PATH];
	int srcIdx;
	CWnd *pWnd;
	pWnd = GetDlgItem(IDC_LIST_SOURCES);
	srcIdx = (int) pWnd->SendMessage(LB_GETCURSEL, 0, (LPARAM)0);
	GetDlgItemText(IDC_EDIT_SRC_Z, buffer, MAX_PATH);
    sscanf_s(buffer, "%f", &srcZ[srcIdx]);
}


void CRoomAcousticsDlg::OnBnClickedCheckSrc1TrackHead()
{
    src1TrackHeadPos = IsDlgButtonChecked(IDC_CHECK_SRC_TRACK_HEAD);
    if (src1TrackHeadPos) {
        CWnd* pWnd = GetDlgItem(IDC_EDIT_SRC_X);
        pWnd->EnableWindow(FALSE);
        pWnd = GetDlgItem(IDC_EDIT_SRC_Y);
        pWnd->EnableWindow(FALSE);
        pWnd = GetDlgItem(IDC_EDIT_SRC_Z);
        pWnd->EnableWindow(FALSE);
    }
    else {
        CWnd* pWnd = GetDlgItem(IDC_EDIT_SRC_X);
        pWnd->EnableWindow(TRUE);
        pWnd = GetDlgItem(IDC_EDIT_SRC_Y);
        pWnd->EnableWindow(TRUE);
        pWnd = GetDlgItem(IDC_EDIT_SRC_Z);
        pWnd->EnableWindow(TRUE);
    }
}


void CRoomAcousticsDlg::OnBnClickedCheckSrc1MuteDirect()
{
    src1MuteDirectPath = IsDlgButtonChecked(IDC_CHECK_SRC_MUTE_DIRECT);
}


void CRoomAcousticsDlg::OnEnKillfocusEditfile2()
{
    GetDlgItemText(IDC_EDITFILE, waveFileNames[sourceIdx], MAX_PATH);
}


void CRoomAcousticsDlg::OnBnClickedCheckEnSrc1mic()
{
    src1EnableMic = IsDlgButtonChecked(IDC_CHECK_EN_SRC);
}


void CRoomAcousticsDlg::OnBnClickedButtonopen2()
{
    CString fileName;
    char * p = fileName.GetBuffer(FILE_LIST_BUFFER_SIZE);
    CFileDialog dlgFile(TRUE);
    OPENFILENAME& ofn = dlgFile.GetOFN();
    ofn.Flags |= OFN_ALLOWMULTISELECT;
    ofn.lpstrFile = p;
    ofn.nMaxFile = FILE_LIST_BUFFER_SIZE;

    dlgFile.DoModal();
    fileName.ReleaseBuffer();

    char * pBufEnd = p + FILE_LIST_BUFFER_SIZE - 2;
    char * start = p;
    while ((p < pBufEnd) && (*p))
        p++;
    if (p > start)
    {
        _tprintf(_T("Path to folder where files were selected:  %s\r\n\r\n"), start);
        p++;
        if (*p == NULL){
			strncpy_s(waveFileNames[sourceIdx], MAX_PATH, start, MAX_PATH);
            SetDlgItemText(IDC_EDITFILE, waveFileNames[sourceIdx]);
        }

        int fileCount = 1;
        while ((p < pBufEnd) && (*p))
        {
            start = p;
            while ((p < pBufEnd) && (*p))
                p++;
            if (p > start)
                _tprintf(_T("%2d. %s\r\n"), fileCount, start);
            p++;
            fileCount++;
        }
    }
}


void CRoomAcousticsDlg::OnBnClickedOk()
{
    CDialog::OnOK();
}

void CRoomAcousticsDlg::OnBnClickedHeadspin()
{
	autoSpinHead = IsDlgButtonChecked(IDC_HEADSPIN);
}



void CRoomAcousticsDlg::OnLbnSelchangeListDevicesRoom()
{
	CWnd *pWnd;
	pWnd = GetDlgItem(IDC_LIST_DEVICES_ROOM);
	
	roomDevIdx = (int) pWnd->SendMessage(LB_GETCURSEL, 0, (LPARAM)0);
	printf("roomDevIdx=%d\n", roomDevIdx);

}


void CRoomAcousticsDlg::OnLbnSelchangeListDevicesConv()
{
	CWnd *pWnd;
	pWnd = GetDlgItem(IDC_LIST_DEVICES_CONV);

	convDevIdx = (int) pWnd->SendMessage(LB_GETCURSEL, 0, (LPARAM)0);
	printf("convDevIdx=%d\n", convDevIdx);
}


void CRoomAcousticsDlg::OnLbnSelchangeListSources()
{
	CWnd *pWnd;
	pWnd = GetDlgItem(IDC_LIST_SOURCES);

	sourceIdx = (int) pWnd->SendMessage(LB_GETCURSEL, 0, (LPARAM)0);
	if (sourceIdx >= nFiles)
		sourceIdx = nFiles - 1;
	updateAllFields();
}


void CRoomAcousticsDlg::OnEnChangeEditNfiles()
{

	nFiles = 0;
    char buffer[MAX_PATH];
	GetDlgItemText(IDC_EDIT_NFILES, buffer, MAX_PATH);
    sscanf_s(buffer, "%d", &nFiles);
	if (nFiles <= 0) nFiles = 1;
	if (nFiles > MAX_SOURCES) nFiles = MAX_SOURCES;
}





