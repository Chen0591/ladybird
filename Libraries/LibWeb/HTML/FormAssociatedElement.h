/*
 * Copyright (c) 2021, Andreas Kling <andreas@ladybird.org>
 * Copyright (c) 2024, Jelle Raaijmakers <jelle@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/String.h>
#include <AK/WeakPtr.h>
#include <LibWeb/Bindings/HTMLFormElementPrototype.h>
#include <LibWeb/DOM/InputEventsTarget.h>
#include <LibWeb/Forward.h>
#include <LibWeb/WebIDL/Types.h>

namespace Web::HTML {

// Form-associated elements should invoke this macro to inject overridden FormAssociatedElement and HTMLElement
// methods as needed. If your class wished to override an HTMLElement method that is overridden here, use the
// following methods instead:
//
//    HTMLElement::inserted() -> Use form_associated_element_was_inserted()
//    HTMLElement::removed_from() -> Use form_associated_element_was_removed()
//
#define FORM_ASSOCIATED_ELEMENT(ElementBaseClass, ElementClass)                                                                                                             \
private:                                                                                                                                                                    \
    virtual HTMLElement& form_associated_element_to_html_element() override                                                                                                 \
    {                                                                                                                                                                       \
        static_assert(IsBaseOf<HTMLElement, ElementClass>);                                                                                                                 \
        return *this;                                                                                                                                                       \
    }                                                                                                                                                                       \
                                                                                                                                                                            \
    virtual void inserted() override                                                                                                                                        \
    {                                                                                                                                                                       \
        ElementBaseClass::inserted();                                                                                                                                       \
        form_node_was_inserted();                                                                                                                                           \
        form_associated_element_was_inserted();                                                                                                                             \
    }                                                                                                                                                                       \
                                                                                                                                                                            \
    virtual void removed_from(DOM::Node* node) override                                                                                                                     \
    {                                                                                                                                                                       \
        ElementBaseClass::removed_from(node);                                                                                                                               \
        form_node_was_removed();                                                                                                                                            \
        form_associated_element_was_removed(node);                                                                                                                          \
    }                                                                                                                                                                       \
                                                                                                                                                                            \
    virtual void attribute_changed(FlyString const& name, Optional<String> const& old_value, Optional<String> const& value, Optional<FlyString> const& namespace_) override \
    {                                                                                                                                                                       \
        ElementBaseClass::attribute_changed(name, old_value, value, namespace_);                                                                                            \
        form_node_attribute_changed(name, value);                                                                                                                           \
        form_associated_element_attribute_changed(name, value);                                                                                                             \
    }

// https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#selection-direction
enum class SelectionDirection {
    Forward,
    Backward,
    None,
};

class FormAssociatedElement {
public:
    HTMLFormElement* form() { return m_form; }
    HTMLFormElement const* form() const { return m_form; }

    void set_form(HTMLFormElement*);

    void element_id_changed(Badge<DOM::Document>);
    void element_with_id_was_added_or_removed(Badge<DOM::Document>);

    bool enabled() const;

    void set_parser_inserted(Badge<HTMLParser>);

    // https://html.spec.whatwg.org/multipage/forms.html#category-listed
    virtual bool is_listed() const { return false; }

    // https://html.spec.whatwg.org/multipage/forms.html#category-submit
    virtual bool is_submittable() const { return false; }

    // https://html.spec.whatwg.org/multipage/forms.html#category-reset
    virtual bool is_resettable() const { return false; }

    // https://html.spec.whatwg.org/multipage/forms.html#category-autocapitalize
    virtual bool is_auto_capitalize_inheriting() const { return false; }

    // https://html.spec.whatwg.org/multipage/forms.html#concept-button
    virtual bool is_button() const { return false; }

    // https://html.spec.whatwg.org/multipage/forms.html#concept-submit-button
    virtual bool is_submit_button() const { return false; }

    virtual String value() const { return String {}; }

    virtual HTMLElement& form_associated_element_to_html_element() = 0;
    HTMLElement const& form_associated_element_to_html_element() const { return const_cast<FormAssociatedElement&>(*this).form_associated_element_to_html_element(); }

    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#concept-form-reset-control
    virtual void reset_algorithm() { }

    virtual void clear_algorithm();

    String form_action() const;
    WebIDL::ExceptionOr<void> set_form_action(String const&);

protected:
    FormAssociatedElement() = default;
    virtual ~FormAssociatedElement() = default;

    virtual void form_associated_element_was_inserted() { }
    virtual void form_associated_element_was_removed(DOM::Node*) { }
    virtual void form_associated_element_attribute_changed(FlyString const&, Optional<String> const&) { }

    void form_node_was_inserted();
    void form_node_was_removed();
    void form_node_attribute_changed(FlyString const&, Optional<String> const&);

private:
    void reset_form_owner();

    WeakPtr<HTMLFormElement> m_form;

    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#parser-inserted-flag
    bool m_parser_inserted { false };
};

enum class SelectionSource {
    UI,
    DOM,
};

class FormAssociatedTextControlElement : public FormAssociatedElement
    , public InputEventsTarget {
public:
    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#concept-textarea/input-relevant-value
    virtual String relevant_value() = 0;
    virtual WebIDL::ExceptionOr<void> set_relevant_value(String const&) = 0;

    virtual void set_dirty_value_flag(bool flag) = 0;

    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-textarea/input-select
    WebIDL::ExceptionOr<void> select();

    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-textarea/input-selectionstart
    Optional<WebIDL::UnsignedLong> selection_start() const;
    WebIDL::ExceptionOr<void> set_selection_start(Optional<WebIDL::UnsignedLong> const&);

    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-textarea/input-selectionend
    Optional<WebIDL::UnsignedLong> selection_end() const;
    WebIDL::ExceptionOr<void> set_selection_end(Optional<WebIDL::UnsignedLong> const&);

    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-textarea/input-selectiondirection
    Optional<String> selection_direction() const;
    void set_selection_direction(Optional<String> direction);
    WebIDL::ExceptionOr<void> set_selection_direction_binding(Optional<String> direction);
    SelectionDirection selection_direction_state() const { return m_selection_direction; }

    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-textarea/input-setrangetext
    WebIDL::ExceptionOr<void> set_range_text(String const& replacement);
    WebIDL::ExceptionOr<void> set_range_text(String const& replacement, WebIDL::UnsignedLong start, WebIDL::UnsignedLong end, Bindings::SelectionMode = Bindings::SelectionMode::Preserve);

    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-textarea/input-setselectionrange
    void set_the_selection_range(Optional<WebIDL::UnsignedLong> start, Optional<WebIDL::UnsignedLong> end, SelectionDirection direction = SelectionDirection::None, SelectionSource source = SelectionSource::DOM);

    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-textarea/input-setselectionrange
    WebIDL::ExceptionOr<void> set_selection_range(Optional<WebIDL::UnsignedLong> start, Optional<WebIDL::UnsignedLong> end, Optional<String> direction);

    // https://w3c.github.io/selection-api/#dfn-has-scheduled-selectionchange-event
    bool has_scheduled_selectionchange_event() const { return m_has_scheduled_selectionchange_event; }
    void set_scheduled_selectionchange_event(bool value) { m_has_scheduled_selectionchange_event = value; }

    virtual void did_edit_text_node() = 0;

    virtual GC::Ptr<DOM::Text> form_associated_element_to_text_node() = 0;
    virtual GC::Ptr<DOM::Text const> form_associated_element_to_text_node() const { return const_cast<FormAssociatedTextControlElement&>(*this).form_associated_element_to_text_node(); }

    virtual void handle_insert(String const&) override;
    virtual void handle_delete(DeleteDirection) override;
    virtual void handle_return_key() override;
    virtual void select_all() override;
    virtual void set_selection_anchor(GC::Ref<DOM::Node>, size_t offset) override;
    virtual void set_selection_focus(GC::Ref<DOM::Node>, size_t offset) override;
    virtual void move_cursor_to_start(CollapseSelection) override;
    virtual void move_cursor_to_end(CollapseSelection) override;
    virtual void increment_cursor_position_offset(CollapseSelection) override;
    virtual void decrement_cursor_position_offset(CollapseSelection) override;
    virtual void increment_cursor_position_to_next_word(CollapseSelection) override;
    virtual void decrement_cursor_position_to_previous_word(CollapseSelection) override;

    GC::Ptr<DOM::Position> cursor_position() const;

protected:
    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#concept-textarea/input-relevant-value
    void relevant_value_was_changed();

private:
    void collapse_selection_to_offset(size_t);
    void selection_was_changed();

    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#concept-textarea/input-selection
    WebIDL::UnsignedLong m_selection_start { 0 };
    WebIDL::UnsignedLong m_selection_end { 0 };
    SelectionDirection m_selection_direction { SelectionDirection::None };

    // https://w3c.github.io/selection-api/#dfn-has-scheduled-selectionchange-event
    bool m_has_scheduled_selectionchange_event { false };
};

}
