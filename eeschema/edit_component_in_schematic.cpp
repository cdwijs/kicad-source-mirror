/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2008 Wayne Stambaugh <stambaughw@gmail.com>
 * Copyright (C) 2004-2018 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

/**
 * @file edit_component_in_schematic.cpp
 * @brief Schematic component editing code.
 */

#include <fctsys.h>
#include <gr_basic.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <sch_edit_frame.h>
#include <msgpanel.h>

#include <general.h>
#include <class_library.h>
#include <sch_component.h>
#include <symbol_lib_table.h>

#include <dialog_edit_one_field.h>


void SCH_EDIT_FRAME::EditComponentFieldText( SCH_FIELD* aField )
{
    wxCHECK_RET( aField != NULL && aField->Type() == SCH_FIELD_T,
                 wxT( "Cannot edit invalid schematic field." ) );

    SCH_COMPONENT* component = (SCH_COMPONENT*) aField->GetParent();

    wxCHECK_RET( component != NULL && component->Type() == SCH_COMPONENT_T,
                 wxT( "Invalid schematic field parent item." ) );

    // Save old component in undo list if not already in edit, or moving.
    int mask = EDA_ITEM_ALL_FLAGS - ( SELECTED | HIGHLIGHTED | BRIGHTENED );
    if( ( aField->GetFlags() & mask ) == 0 )    // i.e. not edited, or moved
        SaveCopyInUndoList( component, UR_CHANGED );

    // Don't use GetText() here.  If the field is the reference designator and it's parent
    // component has multiple parts, we don't want the part suffix added to the field.
    m_canvas->SetIgnoreMouseEvents( true );

    wxString title;
    title.Printf( _( "Edit %s Field" ), GetChars( aField->GetName() ) );

    DIALOG_SCH_EDIT_ONE_FIELD dlg( this, title, aField );

    // The dialog may invoke a kiway player for footprint fields
    // so we must use a quasimodal
    if( dlg.ShowQuasiModal() != wxID_OK )
    {
        m_canvas->MoveCursorToCrossHair();
        m_canvas->SetIgnoreMouseEvents( false );
        return;
    }

    dlg.UpdateField( aField, m_CurrentSheet );
    m_canvas->MoveCursorToCrossHair();
    m_canvas->SetIgnoreMouseEvents( false );
    OnModify();

    if( m_autoplaceFields )
        component->AutoAutoplaceFields( GetScreen() );

    m_canvas->Refresh();

    MSG_PANEL_ITEMS items;
    component->SetCurrentSheetPath( &GetCurrentSheet() );
    component->GetMsgPanelInfo( m_UserUnits, items );
    SetMsgPanel( items );
}


void SCH_EDIT_FRAME::RotateField( SCH_FIELD* aField )
{
    wxCHECK_RET( aField != NULL && aField->Type() == SCH_FIELD_T && !aField->GetText().IsEmpty(),
                 wxT( "Cannot rotate invalid schematic field." ) );

    SCH_COMPONENT* component = (SCH_COMPONENT*) aField->GetParent();

    // Save old component in undo list if not already in edit, or moving.
    int mask = EDA_ITEM_ALL_FLAGS - ( SELECTED | HIGHLIGHTED | BRIGHTENED );
    if( ( aField->GetFlags() & mask ) == 0 )
        SaveCopyInUndoList( component, UR_CHANGED );

    if( aField->GetTextAngle() == TEXT_ANGLE_HORIZ )
        aField->SetTextAngle( TEXT_ANGLE_VERT );
    else
        aField->SetTextAngle( TEXT_ANGLE_HORIZ );

    OnModify();
}
