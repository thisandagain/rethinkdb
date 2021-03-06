# Copyright 2010-2012 RethinkDB, all rights reserved.
class Walkthrough extends Backbone.View
    template: Handlebars.templates['walkthrough_popup-template']
    modal_template: Handlebars.templates['walkthrough_modal-template']

    events: ->
        'click .show-walkthrough a': 'show_walkthrough'
        'click .hide-popup a': 'hide_popup'

    render: =>
        @.$el.html @template
        return @

    hide_popup: (event) =>
        event.preventDefault()
        @.$el.remove()

    show_walkthrough: (event) =>
        event.preventDefault()
        @.$el.remove()
        $walkthrough = $('#walkthrough').html @modal_template

        $('.modal', $walkthrough).modal('show')
        $('.carousel', $walkthrough).carousel
