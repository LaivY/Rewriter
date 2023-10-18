@ECHO OFF

:: ������Ʈ ���� ��η� �����ؾ� ��
SET ROOT=C:\Dev\Rewriter

:: ���������� ���� Ŭ���̾�Ʈ ������ ������ ����
SET COMMON=

:: Ŭ���̾�Ʈ ������ ���� ��ũ
IF EXIST %ROOT%\Game\Client\Data (
	RMDIR /S /Q %ROOT%\Game\Client\Data
)
MKLINK /D %ROOT%\Game\Client\Data %ROOT%\Data

:: ���� ������ ������ �������� �ʿ��� Ŭ���̾�Ʈ ���� ��ũ
FOR %%f IN (%COMMON%) DO (
	IF EXIST %ROOT%\DataSvr\%%f (
		RMDIR /S /Q %ROOT%\DataSvr\%%f
	)
	MKLINK /D %ROOT%\DataSvr\%%f %ROOT%\Data\%%f
)

:: �α��� ���� ������ ���� ��ũ
IF EXIST %ROOT%\Game\LoginServer\DataSvr (
	RMDIR /S /Q %ROOT%\Game\LoginServer\DataSvr
)
MKLINK /D %ROOT%\Game\LoginServer\DataSvr %ROOT%\DataSvr

PAUSE