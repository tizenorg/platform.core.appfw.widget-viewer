# for i18n

SET(POFILES
	ar.po
	as.po
	bn.po
	bn_BD.po
	en.po
	en_PH.po
	en_US.po
	es_US.po
	fa.po
	fr.po
	gu.po
	hi.po
	id.po
	km.po
	kn.po
	ko_KR.po
	lo.po
	ml.po
	mr.po
	ms.po
	my_ZG.po
	ne.po
	or.po
	pa.po
	pt_BR.po
	pt_PT.po
	ru_RU.po
	si.po
	ta.po
	te.po
	th.po
	tl.po
	tr_TR.po
	ur.po
	vi.po
	zh_CN.po
)

SET(MSGFMT "/usr/bin/msgfmt")
FOREACH(pofile ${POFILES})
	SET(pofile ${CMAKE_CURRENT_SOURCE_DIR}/${pofile})
	MESSAGE(STATUS "  ${pofile}")
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

ADD_CUSTOM_TARGET(po ALL DEPENDS ${moFiles})
