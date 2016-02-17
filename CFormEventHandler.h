#pragma once
#define BUFSIZE 32

#include <list>
#include <Windows.h>

typedef struct customForm
{
	int _submitId;						// submit button�� ID
	char _method[BUFSIZE];				
	char _action[BUFSIZE];

	HWND _submitButton;					// submit button�� handle
	std::list<HWND> _hEdits;			// <form>�� ���ԵǴ� edit controls
	std::list<char*> _names;			// edit control���� �ش��ϴ� name.
}customForm;

class CFormEventHandler
{
private:
	int m_formFlag;			// ���� <form> ��������?
	int m_numControl;		// control�� �� ���� : ID�� �������ֱ� ���ؼ�.
	char* m_bodyData;		// POST request�� �ʿ��� body ������
	char* m_queryString;	// GET request�� �ʿ��� queryString 
	char* m_action;			// submit button�� ������ �ִ� action. -> submit button�� �������� �� �� ������ action�� �Ҵ��.

	std::list<customForm> m_forms;	// <form> �±׿� ���� ����ü
	std::list<int> m_submitIds;		// ���� submit button ID�� ������. -> event �߻��� �� ���� Ȯ��.

private:
	bool GetControlInfo(char* inputTag, char* typeBuf, char* nameBuf);		// ex)<input text= name= > ���� text�� name�� �Ľ�

public:
	CFormEventHandler();
	~CFormEventHandler();
	void Cleanup();

	void SetFormFlag(bool flag);													// <form> �±װ� ���� �� true,
																					// </form> �±��� ��� false.
																					// <form> </form> ���̿� ���� control���� ���� form struct�� ����.
	bool MakeEventForm(char* formTag);												// <form>�� ���� �� ������ �� control���� ������ struct ����
	HWND RegisterFormControls(char* inputTag, HWND hParent, HINSTANCE hInstance);	// form struct�� control���� ����

	int EventProc(WPARAM wParam);													// submit button�� ������ �� event ó��
	int GetNumForms();																// �� control ���� ��ȯ.
	bool MakePostBody(customForm iter);												// POST request �� body ����
	char* GetBodyData();
	int GetBodyLength();														
	char* GetAction();
	bool IsActionPushed(WPARAM wParam);												// submit button�� ���ȴ��� Ȯ��
};