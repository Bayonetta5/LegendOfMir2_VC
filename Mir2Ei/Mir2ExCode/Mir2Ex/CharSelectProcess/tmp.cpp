  switch(tdm.wIdent)
    {
    case SM_QUERYCHR:
      m_SelectChr = -1;
      if (tdm.nRecog)
        {
        char szDecodeMsg[512];
        char *pszID = NULL, *pszJob = NULL, *pszFace = NULL, *pszLevel = NULL, *pszSex = NULL, *pszNext = NULL;
        int nPos = 0;
        m_SelectChr = 0;
        cbSelectChar->Clear();
        ZeroMemory(szDecodeMsg, sizeof(szDecodeMsg));
        fnDecode6BitBuf((pszFirst + 1 + DEFBLOCKSIZE), szDecodeMsg, sizeof(szDecodeMsg));
        m_ChrCount = tdm.nRecog;
        for (int i = 0; i < tdm.nRecog; i++)
          {
          if (szDecodeMsg[nPos] == '*')
            {
            pszID = &szDecodeMsg[nPos + 1];
            m_SelectChr = i;
            }
          else
            pszID = &szDecodeMsg[nPos];
          if (pszJob = strchr(pszID, '/'))
            {
            *pszJob = '\0';
            pszJob++;
            if (pszFace = strchr(pszJob, '/'))
              {
              *pszFace = '\0';
              pszFace++;
              if (pszLevel = strchr(pszFace, '/'))
                {
                *pszLevel = '\0';
                pszLevel++;
                if (pszSex = strchr(pszLevel, '/'))
                  {
                  *pszSex = '\0';
                  pszSex++;
                  cbSelectChar->Items->Add(AnsiString(pszID)+","+AnsiString(pszSex)+AnsiString(pszJob)+","+AnsiString(pszLevel)+"¼¶");
                  if (pszNext = strchr(pszSex, '/'))
                    {
                    pszNext++;
                    if (*pszNext != '\0')
                      nPos = pszNext - &szDecodeMsg[0];
                    }
                  }
                }
              }
            } // For
            SelChrTitle->Caption=AnsiString(m_SelectChr);
          }
        }
      break;
    case SM_NEWCHR_SUCCESS: // New character Create Success
      break;
    case SM_STARTPLAY: // Start Game
    break;
    }
