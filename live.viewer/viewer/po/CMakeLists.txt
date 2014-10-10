# for i18n

SET(POFILES
da.po et.po gl.po is.po lo.po nb.po ro.po ta.po zh_CN.po
ar.po de_DE.po eu.po gu.po it_IT.po lt.po ne.po ru_RU.po te.po zh_HK.po
as.po el_GR.po fa.po he.po ja_JP.po lv.po nl_NL.po sinhala.po th.po zh_TW.po
az.po en.po fi.po hi.po ka.po mk.po or.po sk.po tr_TR.po
bg.po en_PH.po fil.po hr.po khmer.po ml.po pa.po sl.po uk.po
bn.po en_US.po fr_CA.po hu.po kk.po mr.po pl.po sq.po ur.po
ca.po es_ES.po fr_FR.po hy.po kn.po ms.po pt_BR.po sr.po uz.po
cs.po es_MX.po ga.po id.po ko_KR.po my.po pt_PT.po sv.po vi.po
)

SET(MSGFMT "/usr/bin/msgfmt")

FOREACH(pofile ${POFILES})
	SET(pofile ${CMAKE_CURRENT_SOURCE_DIR}/${pofile})
	MESSAGE("PO: ${pofile}")
	GET_FILENAME_COMPONENT(absPofile ${pofile} ABSOLUTE)
	GET_FILENAME_COMPONENT(lang ${absPofile} NAME_WE)
	SET(moFile ${CMAKE_CURRENT_BINARY_DIR}/${lang}.mo)
	ADD_CUSTOM_COMMAND(
			OUTPUT ${moFile}
			COMMAND ${MSGFMT} -o ${moFile} ${absPofile}
			DEPENDS ${absPofile}
	)
	INSTALL(FILES ${moFile}
			DESTINATION ${LOCALEDIR}/${lang}/LC_MESSAGES RENAME ${PROJECT_NAME}.mo)
	SET(moFiles ${moFiles} ${moFile})
ENDFOREACH(pofile)

MESSAGE(".mo files: ${moFiles}")
ADD_CUSTOM_TARGET(po ALL DEPENDS ${moFiles})
